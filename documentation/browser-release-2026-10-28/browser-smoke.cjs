// Serve runtime on port 8766; run using Electron with ELECTRON_RUN_AS_NODE unset.
const {app, BrowserWindow} = require('electron');
const fs = require('fs');
const path = require('path');
const sleep = ms => new Promise(r => setTimeout(r, ms));
const report = {checks: [], console: []};
app.setPath('userData', '/tmp/tankgame-release-profile');
app.whenReady().then(async () => {
    const win = new BrowserWindow({width:1100,height:800});
    const wc = win.webContents;
    const js = code => wc.executeJavaScript(code);
    wc.on('console-message', (_event, level, message) => report.console.push({level,message}));
    const watchdog = setTimeout(() => {
        report.error = 'Timed out';
        fs.writeFileSync(path.join(__dirname,'browser-smoke.json'),JSON.stringify(report,null,2));
        app.exit(2);
    }, 90000);
    async function check(label, expression) {
        const value = await js(expression);
        report.checks.push({label, value});
        process.stdout.write(label + ': ' + JSON.stringify(value) + '\n');
        if (!value) throw Error(label);
    }
    async function key(code) {
        wc.sendInputEvent({type:'keyDown',keyCode:code}); await sleep(80);
        wc.sendInputEvent({type:'keyUp',keyCode:code}); await sleep(250);
    }
    async function click(selector) {
        const p = await js(`(() => {const r=document.querySelector('${selector}').getBoundingClientRect();return {x:Math.round(r.x+r.width/2),y:Math.round(r.y+r.height/2)};})()`);
        wc.sendInputEvent({type:'mouseDown',button:'left',clickCount:1,...p});
        wc.sendInputEvent({type:'mouseUp',button:'left',clickCount:1,...p}); await sleep(500);
    }
    try {
        await wc.session.clearCache();
        await win.loadURL('http://127.0.0.1:8766/tankgame-linux.html');
        win.focus(); wc.focus();
        for(let i=0;i<300;++i) {
            if(await js("document.querySelector('#output').value.includes('Browser first frame completed')")) break;
            await sleep(100);
        }
        await check('startup', "!!Module.tankgameActivity");
        await check('all levels packaged', "FS.readdir('/levels').filter(x=>x.endsWith('.txt')).length===22");
        await check('audio assets packaged', "FS.readdir('/sounds').filter(x=>x.endsWith('.wav')).length===10");
        await check('sound requires activation', '!Module.tankgameActivity.audioEnabled');
        await js(`window.realResume=Module.SDL2.audioContext.resume.bind(Module.SDL2.audioContext); Module.SDL2.audioContext.resume=()=>Promise.reject(Error('denied')); document.querySelector('#tankgame-audio').click()`);
        await sleep(100);
        await check('audio rejection retry', "document.querySelector('#tankgame-audio').textContent.includes('retry')");
        await js('void (Module.SDL2.audioContext.resume=window.realResume)');
        await click('#tankgame-audio');
        await check('audio active', "Module.tankgameActivity.audioEnabled && Module.SDL2.audioContext.state==='running'");
        await js(`window.analyser=Module.SDL2.audioContext.createAnalyser();Module.SDL2.audio.scriptProcessorNode.connect(analyser);void 0`);
        await sleep(500);
        await check('OGG produces audio samples', `(() => {const samples=new Float32Array(analyser.fftSize);analyser.getFloatTimeDomainData(samples);return samples.some(x=>Math.abs(x)>.0001)})()`);
        await key('Return');
        await js(`window.g=Module.canvas.getContext('webgl');window.frameCount=0;window.clear=g.clear.bind(g);g.clear=(...a)=>{++frameCount;return clear(...a)};window.loss=g.getExtension('WEBGL_lose_context');void 0`);
        await sleep(300);
        for(let i=0;i<2;++i) {
            await js('loss.loseContext()'); await sleep(400);
            await check('context paused '+i, 'Module.tankgameActivity.contextLost && !Module.tankgameActivity.status.hidden');
            const count=await js('frameCount');await sleep(200);
            await check('no lost-context frames '+i, 'frameCount==='+count);
            await js('loss.restoreContext()'); await sleep(1200);
            await check('context recovered '+i, '!Module.tankgameActivity.contextLost && !Module.tankgameActivity.restore && !g.isContextLost() && g.getError()===0');
            await check('rendering resumed '+i, 'frameCount>'+count);
        }
        await js(`window.energy=0;window.draw=g.drawArrays.bind(g);g.drawArrays=(mode,...args)=>{
            if(mode===g.TRIANGLES && !g.isEnabled(g.DEPTH_TEST)) {
                const p=g.getParameter(g.CURRENT_PROGRAM),m=g.getUniform(p,g.getUniformLocation(p,'uMvp'));
                if(Math.abs(m[12]+.95)<.001 && Math.abs(m[13]-.85)<.001) energy=m[0];
            }return draw(mode,...args);
        };
        window.realPads=navigator.getGamepads.bind(navigator);
        window.pad={id:'Standard test pad',mapping:'standard',index:0,connected:true,timestamp:1,axes:[0,0,0,0],buttons:Array.from({length:16},()=>({pressed:false,touched:false,value:0}))};
        navigator.getGamepads=()=>[pad];
        window.padEvent=type=>{const e=new Event(type);Object.defineProperty(e,'gamepad',{value:pad});window.dispatchEvent(e)};
        padEvent('gamepadconnected');void 0`);
        await sleep(500);
        const beforePad=await js('energy');
        await js('pad.buttons[0]={pressed:true,touched:true,value:1};pad.timestamp++;void 0');
        await sleep(650);
        await check('hotplug gamepad fires', 'energy < '+(beforePad-.001));
        await js("padEvent('gamepaddisconnected');navigator.getGamepads=()=>[];void 0");
        await sleep(500);
        await check('disconnect keeps rendering healthy', 'g.getError()===0');
        await js("navigator.getGamepads=()=>[pad];pad.timestamp++;padEvent('gamepadconnected');void 0");
        await sleep(400);
        const heldEnergy=await js('energy');await sleep(400);
        await check('reconnected held button suppressed', 'energy >= '+(heldEnergy-.001));
        await js('pad.buttons[0]={pressed:false,touched:false,value:0};pad.timestamp++;void 0');await sleep(300);
        const releasedEnergy=await js('energy');
        await js('pad.buttons[0]={pressed:true,touched:true,value:1};pad.timestamp++;void 0');await sleep(600);
        await check('fresh gamepad press after reconnect fires', 'energy < '+(releasedEnergy-.001));
        await js("padEvent('gamepaddisconnected');navigator.getGamepads=realPads;void 0");
        await key('T');
        await check('next level loaded', "document.querySelector('#output').value.includes('level1@@')");
        win.setSize(700,520);await sleep(500);
        await check('responsive resize', 'Math.abs(Module.canvas.getBoundingClientRect().width-innerWidth)<2 && Module.canvas.height===Math.floor((innerHeight-112)*devicePixelRatio)');
        wc.setZoomFactor(1.25);await sleep(500);
        await check('DPI resize', 'Math.abs(Module.canvas.getBoundingClientRect().width*devicePixelRatio-Module.canvas.width)<2 && g.getError()===0');
        wc.setZoomFactor(1);await sleep(300);
        await click('#tankgame-fullscreen');
        await check('fullscreen', 'document.fullscreenElement===Module.canvas.parentElement');
        await click('#tankgame-fullscreen');
        await js(`window.realFocus=document.hasFocus.bind(document);document.hasFocus=()=>false;window.dispatchEvent(new Event('blur'))`);await sleep(250);
        await check('blur suspends audio', "Module.SDL2.audioContext.state==='suspended'");
        await js(`document.hasFocus=window.realFocus;window.dispatchEvent(new Event('focus'))`);await sleep(250);
        await check('focus resumes audio', "Module.SDL2.audioContext.state==='running'");
        await js('window.uploadCount=0;window.upload=g.bufferData.bind(g);g.bufferData=(...a)=>{++uploadCount;return upload(...a)};void 0');
        await sleep(10000);
        await check('steady play has no buffer uploads or GL errors', 'uploadCount===0 && g.getError()===0');
        fs.writeFileSync(path.join(__dirname,'browser-playing.png'),(await wc.capturePage()).toPNG());
        await key('Escape'); await key('Escape');
        await check('shutdown', '!Module.tankgameActivity && !document.querySelector("#tankgame-audio")');
        await check('audio closed', '!Module.SDL2.audioContext');
        process.stdout.write('Checking download failure\n');
        wc.session.webRequest.onBeforeRequest({urls:['http://127.0.0.1:8766/*.js']}, (_details, callback)=>callback({cancel:true}));
        wc.reloadIgnoringCache(); await sleep(2500);
        process.stdout.write('Checking retry UI\n');
        await check('download failure offers retry', '!document.querySelector("#retry").hidden');
        fs.writeFileSync(path.join(__dirname,'browser-download-retry.png'),(await wc.capturePage()).toPNG());
        wc.session.webRequest.onBeforeRequest(null);
        await click('#retry');
        for(let i=0;i<300;++i) {
            if(await js("document.querySelector('#output').value.includes('Browser first frame completed')")) break;
            await sleep(100);
        }
        await check('download retry starts game', '!!Module.tankgameActivity && document.querySelector("#retry").hidden');
        await js(`Module.canvas.focus();window.g=Module.canvas.getContext('webgl');window.loss=g.getExtension('WEBGL_lose_context');g.createShader=()=>null;loss.loseContext();void 0`);
        await sleep(300);
        await js('loss.restoreContext()'); await sleep(1000);
        await check('failed graphics recovery offers retry and stops tasks', '!Module.tankgameActivity && !document.querySelector("#retry").hidden && !Module.SDL2.audioContext');
    } catch(error) { report.error=error.stack; }
    clearTimeout(watchdog);
    fs.writeFileSync(path.join(__dirname,'browser-smoke.json'), JSON.stringify(report,null,2));
    app.exit(report.error?1:0);
});
