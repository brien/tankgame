// Serve runtime/ on port 8766; run with a standalone Electron executable.
const {app, BrowserWindow} = require('electron');
const fs = require('fs');
const path = require('path');
app.setPath('userData', '/tmp/tankgame-resize-browser-profile');
const report = {probes: [], errors: []};
const sleep = ms => new Promise(resolve => setTimeout(resolve, ms));
app.whenReady().then(async () => {
    const win = new BrowserWindow({width: 1200, height: 1000,
        webPreferences: {backgroundThrottling: false}});
    const wc = win.webContents;
    const evaluate = js => wc.executeJavaScript(js);
    const watchdog = setTimeout(() => app.exit(2), 60000);
    try {
        await win.loadURL('about:blank');
        // SDL detects externally sized canvases at window creation. Model a CSS
        // host before startup; the stock generated shell has no responsive CSS.
        wc.debugger.attach('1.3');
        await wc.debugger.sendCommand('Page.enable');
        await wc.debugger.sendCommand('Page.addScriptToEvaluateOnNewDocument', {source: `
            const observer = new MutationObserver(() => {
                const canvas = document.querySelector('#canvas');
                if (canvas) {
                    canvas.style.width = '801px'; canvas.style.height = '601px';
                    observer.disconnect();
                }
            });
            observer.observe(document, {childList: true, subtree: true});
        `});
        console.log('Loading game');
        await win.loadURL('http://127.0.0.1:8766/tankgame-linux.html');
        console.log('Loaded');
        win.focus(); wc.focus();
        for (let i = 0; i < 100; ++i) {
            if (await evaluate("document.querySelector('#output').value.includes('Browser first frame completed')")) break;
            await sleep(100);
        }
        await evaluate("document.querySelector('#canvas').focus()");
        wc.sendInputEvent({type: 'keyDown', keyCode: 'Return'});
        await sleep(100);
        wc.sendInputEvent({type: 'keyUp', keyCode: 'Return'});
        await sleep(1000);
        for (const [width, height, dpr] of [[801,601,1], [997,503,1], [801,601,2], [801,601,1]]) {
            console.log('Resizing', width, height, dpr);
            await wc.debugger.sendCommand('Emulation.setDeviceMetricsOverride', {
                width: 1200, height: 1000, deviceScaleFactor: dpr, mobile: false
            });
            await evaluate(`(() => {
                const c = document.querySelector('#canvas');
                c.style.width = '${width}px'; c.style.height = '${height}px';
                window.dispatchEvent(new Event('resize'));
            })()`);
            await sleep(500);
            const probe = await evaluate(`(() => {
                const c = document.querySelector('#canvas'), g = c.getContext('webgl');
                return {width: c.width, height: c.height, dpr: devicePixelRatio,
                    viewport: Array.from(g.getParameter(g.VIEWPORT)),
                    error: g.getError(), lost: g.isContextLost()};
            })()`);
            report.probes.push(probe);
            if (probe.width !== width*dpr || probe.height !== height*dpr ||
                probe.viewport.join(',') !== [0,0,probe.width,probe.height].join(',') ||
                probe.error || probe.lost) throw Error('Resize probe failed: '+JSON.stringify(probe));
            const png = await wc.capturePage();
            fs.writeFileSync(path.join(__dirname, `browser-${width}-${height}-${dpr}.png`), png.toPNG());
        }
        report.output = await evaluate("document.querySelector('#output').value");
        if (!report.output.includes('GameTask: Game setup complete')) throw Error('Gameplay did not start');
        if (/RuntimeError|memory access out of bounds|shader.*fail/i.test(report.output)) throw Error('Runtime error');
    } catch (error) { report.errors.push(error.stack); }
    finally {
        fs.writeFileSync(path.join(__dirname, 'browser-smoke.json'), JSON.stringify(report, null, 2));
        console.log(JSON.stringify({probes: report.probes, errors: report.errors}));
        clearTimeout(watchdog);
        win.destroy(); app.exit(report.errors.length ? 1 : 0);
    }
});
