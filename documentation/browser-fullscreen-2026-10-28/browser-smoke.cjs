// Serve runtime/ on 8766. Run with Electron and ELECTRON_RUN_AS_NODE unset.
const {app, BrowserWindow} = require('electron');
const fs = require('fs');
const path = require('path');
const sleep = ms => new Promise(resolve => setTimeout(resolve, ms));
app.setPath('userData', '/tmp/tankgame-fullscreen-profile');
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
        const original = await evaluate('({width:Module.canvas.width,height:Module.canvas.height})');
        async function click(selector) {
            const point = await evaluate(`(() => {const r=document.querySelector('${selector}').getBoundingClientRect();return {x:Math.round(r.x+r.width/2),y:Math.round(r.y+r.height/2)};})()`);
            wc.sendInputEvent({type:'mouseDown',button:'left',clickCount:1,...point});
            wc.sendInputEvent({type:'mouseUp',button:'left',clickCount:1,...point});
            await sleep(650);
        }
        async function checkFullscreen(label, active) {
            const result = await evaluate(`(() => {
                const c=Module.canvas, g=c.getContext('webgl');
                return {active:document.fullscreenElement===c.parentElement,
                    width:c.width,height:c.height,cssWidth:c.getBoundingClientRect().width,cssHeight:c.getBoundingClientRect().height,dpr:devicePixelRatio,viewport:Array.from(g.getParameter(g.VIEWPORT)),
                    expectedWidth:Math.round(c.parentElement.clientWidth*devicePixelRatio),
                    expectedHeight:Math.round(c.parentElement.clientHeight*devicePixelRatio),
                    captured:document.pointerLockElement===c,
                    controlsVisible:c.parentElement.contains(document.querySelector('#tankgame-fullscreen')) && c.parentElement.contains(document.querySelector('#tankgame-mouse-capture'))};
            })()`);
            report.probes.push({label,...result});
            if(result.active!==active || !result.controlsVisible) throw Error('Fullscreen/control state mismatch');
            const expected=active ? {width:result.expectedWidth,height:result.expectedHeight} : original;
            if(result.width!==expected.width || result.height!==expected.height || result.viewport[2]!==expected.width || result.viewport[3]!==expected.height) throw Error('Drawable/viewport mismatch');
            if(Math.abs(result.cssWidth*result.dpr-result.width)>1 || Math.abs(result.cssHeight*result.dpr-result.height)>1) throw Error('CSS surface does not match drawable');
            await probe(label+'-gl');
            return result;
        }
        await click('#tankgame-fullscreen');
        if((await checkFullscreen('entered',true)).captured) throw Error('Fullscreen captured mouse automatically');
        wc.setZoomFactor(1.25); await sleep(700);
        await checkFullscreen('fullscreen-dpi-resize',true);
        wc.setZoomFactor(1); await sleep(700);
        await checkFullscreen('fullscreen-dpi-restored',true);
        fs.writeFileSync(path.join(__dirname,'browser-fullscreen.png'),(await wc.capturePage()).toPNG());
        await click('#tankgame-mouse-capture');
        if(!await evaluate('document.pointerLockElement===Module.canvas')) throw Error('Fullscreen capture failed');
        await evaluate('document.exitPointerLock()'); await sleep(300);
        await click('#tankgame-fullscreen');
        await checkFullscreen('button-exit',false);
        await evaluate(`(() => {window.savedRequest=Module.canvas.parentElement.requestFullscreen;Module.canvas.parentElement.requestFullscreen=()=>Promise.reject(new Error('denied'));document.querySelector('#tankgame-fullscreen').click();})()`);
        await sleep(200);
        if(!await evaluate("document.querySelector('#tankgame-fullscreen').textContent.includes('retry')")) throw Error('No denial cue');
        await checkFullscreen('denied',false);
        fs.writeFileSync(path.join(__dirname,'browser-retry.png'),(await wc.capturePage()).toPNG());
        await evaluate('void (Module.canvas.parentElement.requestFullscreen=window.savedRequest)');
        await click('#tankgame-fullscreen');
        await checkFullscreen('retry',true);
        await evaluate('document.exitFullscreen()'); await sleep(650);
        await checkFullscreen('external-exit',false);
        // Keyboard activation uses the same user-gesture path as clicking.
        await evaluate("Module.canvas.focus()");
        await key('Tab');
        if(!await evaluate("document.activeElement.id === 'tankgame-fullscreen'")) throw Error('Canvas Tab did not reach fullscreen control');
        await key('Space'); await sleep(500);
        await checkFullscreen('keyboard-entry',true);
        // Shutdown from fullscreen must exit and remove both controls.
        await evaluate('document.exitFullscreen()'); await sleep(650);
        await key('Escape');
        await click('#tankgame-fullscreen');
        await checkFullscreen('title-fullscreen',true);
        await evaluate("Module.canvas.focus(); Module.canvas.dispatchEvent(new KeyboardEvent('keydown',{key:'Escape',code:'Escape',keyCode:27,which:27,bubbles:true}));");
        await sleep(600);
        report.output=await evaluate("document.querySelector('#output').value");
        if(!report.output.includes('Browser frame loop stopped.') || /RuntimeError|memory access out of bounds/i.test(report.output)) throw Error('Shutdown failed');
        if(await evaluate("'tankgameActivity' in Module || !!document.querySelector('#tankgame-mouse-capture') || !!document.querySelector('#tankgame-fullscreen') || !!document.fullscreenElement")) throw Error('Activity listeners not cleaned up');
    } catch(e) {report.errors.push(e.stack);report.output=await evaluate("document.querySelector('#output').value").catch(()=> '');}
    clearTimeout(watchdog);
    fs.writeFileSync(path.join(__dirname,'browser-smoke.json'),JSON.stringify(report,null,2));
    app.exit(report.errors.length?1:0);
});
