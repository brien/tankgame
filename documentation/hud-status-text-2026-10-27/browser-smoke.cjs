// Serve runtime/ on 8766; run with Electron and ELECTRON_RUN_AS_NODE unset.
const {app, BrowserWindow} = require('electron');
const fs = require('fs');
const path = require('path');
const sleep = ms => new Promise(resolve => setTimeout(resolve, ms));
app.setPath('userData', '/tmp/tankgame-hud-status-profile');
const report = {probes: [], errors: []};
app.whenReady().then(async () => {
    const win = new BrowserWindow({width: 1380, height: 940, webPreferences: {backgroundThrottling: false}});
    const wc = win.webContents;
    const evaluate = js => wc.executeJavaScript(js);
    const watchdog = setTimeout(() => app.exit(2), 60000);
    try {
        await win.loadURL('http://127.0.0.1:8766/tankgame-linux.html');
        let ready = false;
        for (let i=0;i<100;++i) {
            ready = await evaluate("document.querySelector('#output').value.includes('Browser first frame completed')");
            if (ready) break;
            await sleep(100);
        }
        if (!ready) throw Error('Game did not initialize');
        win.focus(); wc.focus();
        await evaluate(`(() => {
            document.querySelector('#canvas').focus();
            const g=document.querySelector('#canvas').getContext('webgl');
            window.hudProbe={menu:0,text:0,globalText:0,statusText:0,targeting:0,bars:0,icons:0,uploads:0,textureUploads:0,minEnergyWidth:1,maxEnergyWidth:0};
            const draw=g.drawArrays.bind(g), upload=g.bufferData.bind(g), texture=g.texImage2D.bind(g);
            g.drawArrays=(mode,...args)=>{
                if(mode===g.LINE_LOOP && !g.isEnabled(g.DEPTH_TEST) &&
                    g.getUniform(g.getParameter(g.CURRENT_PROGRAM),g.getUniformLocation(g.getParameter(g.CURRENT_PROGRAM),'uMvp'))[14] > .5) {
                    ++hudProbe.menu;
                    const v=g.getParameter(g.VIEWPORT);
                    if(v[0] || v[1] || v[2]!==g.drawingBufferWidth || v[3]!==g.drawingBufferHeight)
                        throw Error('Menu is not drawable-global');
                }
                if(mode===g.LINES) {
                    const p=g.getParameter(g.CURRENT_PROGRAM);
                    if(g.getUniform(p,g.getUniformLocation(p,'uTargeting'))) {
                        ++hudProbe.targeting;
                        if(!g.isEnabled(g.DEPTH_TEST) || g.getParameter(g.DEPTH_WRITEMASK)) throw Error('Target depth state');
                    }
                }
                if(mode===g.TRIANGLES && !g.isEnabled(g.DEPTH_TEST)) {
                    const p=g.getParameter(g.CURRENT_PROGRAM), m=g.getUniform(p,g.getUniformLocation(p,'uMvp'));
                    if(Math.abs(m[14])<.0001 && g.getUniform(p,g.getUniformLocation(p,'uHasTexture'))) {
                        ++hudProbe.text;
                        if(m[12]<0 && m[13]<-.5) ++hudProbe.statusText;
                        if(m[12]<0) ++hudProbe.globalText;
                        if(g.getParameter(g.DEPTH_WRITEMASK) || !g.isEnabled(g.BLEND)) throw Error('Text pass state');
                    }
                    // Allow the shared ES shader's mediump uniform quantization.
                    if(Math.abs(m[12]+.95)<.001) {
                        ++hudProbe.bars;
                        if(Math.abs(m[13]-.85)<.001) {
                            hudProbe.minEnergyWidth=Math.min(hudProbe.minEnergyWidth,m[0]);
                            hudProbe.maxEnergyWidth=Math.max(hudProbe.maxEnergyWidth,m[0]);
                        }
                    }
                    if(Math.abs(m[12]+.99)<.001) ++hudProbe.icons;
                }
                return draw(mode,...args);
            };
            g.bufferData=(...args)=>{++hudProbe.uploads; return upload(...args);};
            g.texImage2D=(...args)=>{++hudProbe.textureUploads; return texture(...args);};
        })()`);
        async function key(keyCode) {
            wc.sendInputEvent({type:'keyDown',keyCode}); await sleep(100);
            wc.sendInputEvent({type:'keyUp',keyCode}); await sleep(500);
        }
        async function probe(label) {
            const result=await evaluate(`(() => {
                const g=document.querySelector('#canvas').getContext('webgl');
                return {...hudProbe,error:g.getError(),lost:g.isContextLost()};
            })()`);
            report.probes.push({label,...result});
            if(result.error || result.lost) throw Error('WebGL failure');
            return result;
        }
        await sleep(500);
        const title=await probe('title');
        if(!title.menu) throw Error('Missing global menu');
        if(title.bars || title.icons) throw Error('Player HUD leaked into title');
        fs.writeFileSync(path.join(__dirname,'browser-title.png'),(await wc.capturePage()).toPNG());
        await key('I'); await key('Return'); await sleep(1000);
        const playing=await probe('playing');
        if(!playing.text || !playing.globalText || !playing.statusText) throw Error('Missing FPS/debug text');
        if(!playing.targeting || !playing.bars || !playing.icons) throw Error('Missing HUD bars/icons');
        fs.writeFileSync(path.join(__dirname,'browser-playing.png'),(await wc.capturePage()).toPNG());
        const point=await evaluate(`(() => {const r=document.querySelector('#canvas').getBoundingClientRect();return {x:Math.round(r.x+850),y:Math.round(r.y+380)};})()`);
        wc.sendInputEvent({type:'mouseMove',...point});
        wc.sendInputEvent({type:'mouseDown',button:'left',clickCount:1,...point});
        await sleep(2000);
        const firing=await probe('firing');
        fs.writeFileSync(path.join(__dirname,'browser-firing.png'),(await wc.capturePage()).toPNG());
        wc.sendInputEvent({type:'mouseUp',button:'left',clickCount:1,...point});
        await sleep(1000);
        const cached=await probe('cached');
        if(cached.statusText<=playing.statusText || cached.text<=playing.text || cached.globalText<=playing.globalText || cached.targeting<=playing.targeting || cached.bars<=playing.bars || cached.icons<=playing.icons || cached.uploads!==playing.uploads ||
            cached.textureUploads!==playing.textureUploads) throw Error('HUD draw/cache failure');
        if(firing.minEnergyWidth>=playing.minEnergyWidth) throw Error('Firing did not reduce displayed energy');
        await key('Escape'); await key('Escape');
        report.output=await evaluate("document.querySelector('#output').value");
        if(!report.output.includes('GameTask: Game setup complete') ||
            !report.output.includes('Browser frame loop stopped.') ||
            /RuntimeError|memory access out of bounds/i.test(report.output)) throw Error('Gameplay/shutdown failed');
    } catch(e) {report.errors.push(e.stack); report.output=await evaluate("document.querySelector('#output').value").catch(()=> '');}
    clearTimeout(watchdog);
    fs.writeFileSync(path.join(__dirname,'browser-smoke.json'),JSON.stringify(report,null,2));
    app.exit(report.errors.length?1:0);
});
