// Serve runtime on port 8766 and run with Electron (ELECTRON_RUN_AS_NODE unset).
const {app, BrowserWindow} = require('electron');
const fs = require('fs');
const path = require('path');
const sleep = ms => new Promise(resolve => setTimeout(resolve, ms));
const report = {checks: [], errors: []};
app.setPath('userData', '/tmp/tankgame-multiplayer-profile');
app.whenReady().then(async () => {
    const win = new BrowserWindow({width: 1100, height: 800});
    const wc = win.webContents;
    const js = code => wc.executeJavaScript(code);
    wc.on('console-message', (_event, level, message) => {
        if (level >= 3) report.errors.push(message);
    });
    const watchdog = setTimeout(() => app.exit(2), 90000);
    async function key(keyCode) {
        win.focus(); wc.focus();
        await js('Module.canvas.focus(); void 0');
        await sleep(100);
        wc.sendInputEvent({type: 'keyDown', keyCode}); await sleep(150);
        wc.sendInputEvent({type: 'keyUp', keyCode}); await sleep(350);
    }
    async function check(label, expression) {
        const value = await js(expression);
        report.checks.push({label, value});
        if (!value) throw Error(label);
    }
    async function view(label, split) {
        await js('window.views = new Set(); void 0');
        await sleep(400);
        await check(label, `!Module.tankgameActivity.contextLost && g.getError()===0 &&
            ${split ? '' : '!'}Array.from(views).some(v => JSON.parse(v)[3] < g.drawingBufferHeight - 1) && views.size > 0`);
    }
    try {
        await wc.session.clearCache();
        await win.loadURL('http://127.0.0.1:8766/tankgame-linux.html');
        win.focus(); wc.focus();
        for (let i = 0; i < 300; ++i) {
            if (await js("document.querySelector('#output').value.includes('Browser first frame completed')")) break;
            await sleep(100);
        }
        await js(`window.g=Module.canvas.getContext('webgl'); window.views=new Set();
            window.draw=g.drawArrays.bind(g); g.drawArrays=(...args)=>{
                views.add(JSON.stringify(Array.from(g.getParameter(g.VIEWPORT)))); return draw(...args);
            }; void 0`);
        for (const mode of [1, 0, 2, 0, 1, 2, 1, 0]) {
            await key('Left');
            if (mode > 0) await key('Right');
            if (mode === 2) await key('Down');
            await key('Return');
            await view('mode ' + mode + ' viewport', mode > 0);
            if (mode > 0) {
                for (let reload = 0; reload < 3; ++reload) {
                    await key('T');
                    await view('mode ' + mode + ' reload ' + reload, true);
                }
            }
            fs.writeFileSync(path.join(__dirname, 'browser-mode-' + mode + '.png'), (await wc.capturePage()).toPNG());
            await key('Escape');
        }
        await key('Escape');
        await check('clean shutdown', 'Module.tankgameActivity === undefined || Module.tankgameActivity.stopped === true || document.querySelector("#output").value.includes("shutdown")');
        if (report.errors.length) throw Error('Unexpected browser console errors');
    } catch (error) {
        report.error = error.stack;
        report.state = await js(`({views: Array.from(views), height: g.drawingBufferHeight,
            output: document.querySelector('#output').value})`);
        fs.writeFileSync(path.join(__dirname, 'browser-failure.png'), (await wc.capturePage()).toPNG());
    }
    clearTimeout(watchdog);
    fs.writeFileSync(path.join(__dirname, 'browser-smoke.json'), JSON.stringify(report, null, 2));
    app.exit(report.error ? 1 : 0);
});
