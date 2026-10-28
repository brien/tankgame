// Serve runtime/ on 8766. Run with Electron and ELECTRON_RUN_AS_NODE unset.
const {app, BrowserWindow} = require('electron');
const fs = require('fs');
const path = require('path');
const sleep = ms => new Promise(resolve => setTimeout(resolve, ms));
app.setPath('userData', '/tmp/tankgame-pointer-profile');
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
        const capturePoint=await evaluate(`(() => {const r=document.querySelector('#tankgame-mouse-capture').getBoundingClientRect();return {x:Math.round(r.x+r.width/2),y:Math.round(r.y+r.height/2)};})()`);
        async function capture() {
            wc.sendInputEvent({type:'mouseDown',button:'left',clickCount:1,...capturePoint});
            wc.sendInputEvent({type:'mouseUp',button:'left',clickCount:1,...capturePoint});
            await sleep(400);
            if(!await evaluate('document.pointerLockElement === Module.canvas')) throw Error('Capture failed');
        }
        await capture();
        await probe('captured');
        fs.writeFileSync(path.join(__dirname,'browser-captured.png'),(await wc.capturePage()).toPNG());
        wc.sendInputEvent({type:'mouseMove',movementX:50,movementY:10,...point});
        await sleep(200);
        await probe('relative-motion');
        await evaluate('document.exitPointerLock()');
        await sleep(400);
        if(await evaluate('document.pointerLockElement !== null')) throw Error('Release failed');
        await probe('released');
        await sleep(1500);
        await capture();
        await probe('recaptured');
        const other = new BrowserWindow({width:300,height:200});
        await other.loadURL('about:blank'); other.focus();
        await sleep(400);
        const blurred=await probe('blurred');
        await sleep(500);
        if((await probe('blurred-wait')).frames!==blurred.frames) throw Error('Blur ticked');
        win.focus(); wc.focus(); other.close();
        await sleep(500);
        await evaluate('document.exitPointerLock()');
        await sleep(200);
        // Exercise both rejection paths without relying on host permission settings.
        await evaluate(`(() => {window.savedRequest=Module.canvas.requestPointerLock;Module.canvas.requestPointerLock=()=>Promise.reject(new Error('denied'));document.querySelector('#tankgame-mouse-capture').click();})()`);
        await sleep(200);
        if(!await evaluate("document.querySelector('#tankgame-mouse-capture').textContent.includes('retry')")) throw Error('No denial cue');
        await probe('denied-keyboard-fallback');
        fs.writeFileSync(path.join(__dirname,'browser-retry.png'),(await wc.capturePage()).toPNG());
        await evaluate('void (Module.canvas.requestPointerLock=window.savedRequest)');
        await sleep(1500);
        await capture();
        await probe('retry-success');
        await evaluate('document.exitPointerLock()');
        await sleep(300);
        await key('Escape'); await key('Escape');
        report.output=await evaluate("document.querySelector('#output').value");
        if(!report.output.includes('Browser frame loop stopped.') || /RuntimeError|memory access out of bounds/i.test(report.output)) throw Error('Shutdown failed');
        if(await evaluate("'tankgameActivity' in Module || !!document.querySelector('#tankgame-mouse-capture')")) throw Error('Activity listeners not cleaned up');
    } catch(e) {report.errors.push(e.stack);report.output=await evaluate("document.querySelector('#output').value").catch(()=> '');}
    clearTimeout(watchdog);
    fs.writeFileSync(path.join(__dirname,'browser-smoke.json'),JSON.stringify(report,null,2));
    app.exit(report.errors.length?1:0);
});
