// Serve runtime/ on 8766; run using Electron with ELECTRON_RUN_AS_NODE unset.
const {app, BrowserWindow} = require('electron');
const fs = require('fs');
const path = require('path');
const sleep = ms => new Promise(resolve => setTimeout(resolve, ms));
app.setPath('userData', '/tmp/tankgame-menu-browser-profile');
const report = {probes: [], errors: []};
app.whenReady().then(async () => {
    const win = new BrowserWindow({width: 1000, height: 800, webPreferences: {backgroundThrottling: false}});
    const wc = win.webContents;
    const evaluate = js => wc.executeJavaScript(js);
    const watchdog = setTimeout(() => app.exit(2), 60000);
    try {
        await win.loadURL('http://127.0.0.1:8766/tankgame-linux.html');
        for (let i=0;i<100;++i) {
            if (await evaluate("document.querySelector('#output').value.includes('Browser first frame completed')")) break;
            await sleep(100);
        }
        win.focus(); wc.focus();
        await evaluate(`(() => {
            document.querySelector('#canvas').focus();
            const g=document.querySelector('#canvas').getContext('webgl');
            window.menuProbe={draws:0,uploads:0};
            const draw=g.drawArrays.bind(g), upload=g.bufferData.bind(g);
            g.drawArrays=(mode,...args)=>{if(mode===g.LINE_LOOP && !g.isEnabled(g.DEPTH_TEST)) ++menuProbe.draws; return draw(mode,...args);};
            g.bufferData=(...args)=>{++menuProbe.uploads; return upload(...args);};
        })()`);
        async function key(keyCode) {
            wc.sendInputEvent({type:'keyDown',keyCode}); await sleep(100);
            wc.sendInputEvent({type:'keyUp',keyCode}); await sleep(500);
        }
        async function probe(label) {
            const result=await evaluate(`(() => {
                const g=document.querySelector('#canvas').getContext('webgl');
                return {...menuProbe,error:g.getError(),lost:g.isContextLost()};
            })()`);
            report.probes.push({label,...result});
            if(result.error || result.lost) throw Error('WebGL failure');
            return result;
        }
        await sleep(500);
        let previous=await probe('title-0');
        if(!previous.draws) throw Error('No menu draws');
        for (const [keyCode,label] of [['Right','title-1'],['Down','title-2'],['Left','title-0-return']]) {
            await key(keyCode);
            const current=await probe(label);
            if(current.draws<=previous.draws || current.uploads!==previous.uploads) throw Error('Menu draw/cache failure');
            fs.writeFileSync(path.join(__dirname,label+'.png'),(await wc.capturePage()).toPNG());
            previous=current;
        }
        await key('Return');
        await sleep(1000);
        const playing=await probe('playing');
        await sleep(1000);
        const cached=await probe('playing-cached');
        if(cached.draws!==playing.draws || cached.uploads!==playing.uploads) throw Error('Menu remained visible or repeated uploads');
        await key('Escape');
        await sleep(300);
        await key('Escape');
        report.output=await evaluate("document.querySelector('#output').value");
        if(!report.output.includes('GameTask: Game setup complete') || /RuntimeError|memory access out of bounds/i.test(report.output)) throw Error('Gameplay failed');
    } catch(e) {report.errors.push(e.stack);}
    clearTimeout(watchdog);
    fs.writeFileSync(path.join(__dirname,'browser-smoke.json'),JSON.stringify(report,null,2));
    app.exit(report.errors.length?1:0);
});
