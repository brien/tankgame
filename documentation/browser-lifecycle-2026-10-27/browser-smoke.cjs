// Serve runtime/ on 8766. Run with Electron and ELECTRON_RUN_AS_NODE unset.
const {app, BrowserWindow} = require('electron');
const fs = require('fs');
const path = require('path');
const sleep = ms => new Promise(resolve => setTimeout(resolve, ms));
app.setPath('userData', '/tmp/tankgame-lifecycle-profile');
const report = {probes: [], errors: [], console: []};
app.whenReady().then(async () => {
    const win = new BrowserWindow({width: 1380, height: 940});
    const wc = win.webContents;
    wc.on('console-message', (_event, level, message) => report.console.push({level,message}));
    const evaluate = js => wc.executeJavaScript(js);
    const watchdog = setTimeout(() => app.exit(2), 60000);
    try {
        await wc.session.clearCache();
        await win.loadURL('http://127.0.0.1:8766/tankgame-linux.html');
        win.focus(); wc.focus();
        let ready = false;
        for (let i=0;i<300;++i) {
            ready = await evaluate("document.querySelector('#output').value.includes('Browser first frame completed')");
            if (ready) break;
            await sleep(100);
        }
        if (!ready) throw Error('Game did not initialize');
        await evaluate(`(() => {
            document.querySelector('#canvas').focus();
            const g=document.querySelector('#canvas').getContext('webgl');
            window.lifecycleProbe={frames:0,energy:0,uploads:0};
            const clear=g.clear.bind(g), draw=g.drawArrays.bind(g), upload=g.bufferData.bind(g);
            g.clear=(...args)=>{++lifecycleProbe.frames;return clear(...args);};
            g.bufferData=(...args)=>{++lifecycleProbe.uploads;return upload(...args);};
            g.drawArrays=(mode,...args)=>{
                if(mode===g.TRIANGLES && !g.isEnabled(g.DEPTH_TEST)) {
                    const p=g.getParameter(g.CURRENT_PROGRAM),m=g.getUniform(p,g.getUniformLocation(p,'uMvp'));
                    if(Math.abs(m[12]+.95)<.001 && Math.abs(m[13]-.85)<.001) lifecycleProbe.energy=m[0];
                }
                return draw(mode,...args);
            };
        })()`);
        async function key(keyCode) {
            wc.sendInputEvent({type:'keyDown',keyCode}); await sleep(80);
            wc.sendInputEvent({type:'keyUp',keyCode}); await sleep(350);
        }
        async function probe(label) {
            const result=await evaluate(`(() => {
                const g=document.querySelector('#canvas').getContext('webgl');
                return {...lifecycleProbe,hidden:document.hidden,focused:document.hasFocus(),
                    paused:!Module.tankgameActivity.status.hidden,error:g.getError(),lost:g.isContextLost()};
            })()`);
            report.probes.push({label,...result});
            if(result.error || result.lost) throw Error('WebGL failure');
            return result;
        }
        await key('Return'); await sleep(500);
        await probe('playing');
        const point=await evaluate(`(() => {const r=document.querySelector('#canvas').getBoundingClientRect();return {x:Math.round(r.x+400),y:Math.round(r.y+300)};})()`);
        wc.sendInputEvent({type:'keyDown',keyCode:'W'});
        wc.sendInputEvent({type:'mouseDown',button:'left',clickCount:1,...point});
        await sleep(300);
        const other = new BrowserWindow({width:300,height:200});
        await other.loadURL('about:blank'); other.focus();
        await sleep(250);
        const blurred=await probe('blurred');
        if(blurred.focused || !blurred.paused) throw Error('Blur did not pause');
        await sleep(1200);
        const stillBlurred=await probe('blurred-wait');
        if(stillBlurred.frames!==blurred.frames) throw Error('Game ticked while blurred');
        fs.writeFileSync(path.join(__dirname,'browser-paused.png'),(await wc.capturePage()).toPNG());
        win.hide(); await sleep(250);
        const hidden=await probe('hidden');
        await sleep(1200);
        const stillHidden=await probe('hidden-wait');
        if(!hidden.hidden || stillHidden.frames!==hidden.frames) throw Error('Hidden game ticked');
        win.show(); win.focus(); wc.focus(); other.close();
        await sleep(250);
        const resumed=await probe('resumed');
        if(resumed.paused || resumed.frames<=hidden.frames) throw Error('Resume failed');
        await sleep(600);
        const released=await probe('no-stale-fire');
        if(released.energy<resumed.energy-.001) throw Error('Stale mouse input fired after resume');
        wc.sendInputEvent({type:'keyUp',keyCode:'W'});
        wc.sendInputEvent({type:'mouseUp',button:'left',clickCount:1,...point});
        await sleep(200);
        const before=await probe('before-fresh-fire');
        wc.sendInputEvent({type:'mouseDown',button:'left',clickCount:1,...point});
        await sleep(500);
        const fresh=await probe('fresh-fire');
        if(fresh.energy>=before.energy) throw Error('Fresh mouse input did not fire');
        wc.sendInputEvent({type:'mouseUp',button:'left',clickCount:1,...point});
        fs.writeFileSync(path.join(__dirname,'browser-resumed.png'),(await wc.capturePage()).toPNG());
        // A blocked main thread must reset the next simulation delta too.
        await evaluate('(() => {const until=performance.now()+600;while(performance.now()<until){};})()');
        await sleep(1200);
        await probe('long-gap');
        await key('Escape'); await key('Escape');
        report.output=await evaluate("document.querySelector('#output').value");
        if(!report.output.includes('Browser frame loop stopped.') || /RuntimeError|memory access out of bounds/i.test(report.output)) throw Error('Shutdown failed');
        if(await evaluate("'tankgameActivity' in Module")) throw Error('Activity listeners not cleaned up');
    } catch(e) {report.errors.push(e.stack);report.output=await evaluate("document.querySelector('#output').value").catch(()=> '');}
    clearTimeout(watchdog);
    fs.writeFileSync(path.join(__dirname,'browser-smoke.json'),JSON.stringify(report,null,2));
    app.exit(report.errors.length?1:0);
});
