const {app,BrowserWindow}=require('electron');
const fs=require('fs');
const path=require('path');
const mode=process.env.CONFIG_MODE || 'valid';
const output=process.env.CONFIG_OUTPUT || __dirname;
const expected=process.env.CONFIG_EXPECTED;
const report={checks:[],console:[]};
const sleep=ms=>new Promise(r=>setTimeout(r,ms));
app.setPath('userData','/tmp/tankgame-configuration-smoke-profile');
app.whenReady().then(async()=>{
 const win=new BrowserWindow({width:1100,height:800});
 const wc=win.webContents;
 const js=code=>wc.executeJavaScript(code);
 wc.on('console-message',(_event,level,message)=>report.console.push({level,message}));
 const watchdog=setTimeout(()=>{report.error='timeout';finish(2)},45000);
 function finish(code){clearTimeout(watchdog);fs.mkdirSync(output,{recursive:true});fs.writeFileSync(path.join(output,'configuration-smoke.json'),JSON.stringify(report,null,2));app.exit(code)}
 async function check(label,expression){const value=await js(expression);report.checks.push({label,value});if(!value)throw Error(label)}
 try {
  await wc.session.clearCache();
  await win.loadURL('http://127.0.0.1:8766/tankgame-linux.html');
  for(let i=0;i<300;i++){
   if(await js(mode==='invalid'?"!document.querySelector('#retry').hidden":"document.querySelector('#output').value.includes('Browser first frame completed')"))break;
   await sleep(100);
  }
  if(mode==='invalid'){
   await check('visible startup failure',"!document.querySelector('#status').hidden && document.querySelector('#status').textContent.includes('Unable to start game')");
   await check('no frame loop',"!document.querySelector('#output').value.includes('Browser first frame completed')");
   await check('diagnostic identifies configuration',"document.querySelector('#output').value.includes('gameplay-config.json')");
  }else{
   await check('first frame',"document.querySelector('#output').value.includes('Browser first frame completed')");
   await check('selected packaged file',"document.querySelector('#output').value.includes('Gameplay configuration: /gameplay-config.json (schema version 1)')");
   if(expected){const text=fs.readFileSync(expected,'utf8');await check('packaged bytes match source',`FS.readFile('/gameplay-config.json',{encoding:'utf8'})===${JSON.stringify(text)}`)}
   win.focus();wc.focus();await js('Module.canvas.focus(); void 0');
   wc.sendInputEvent({type:'keyDown',keyCode:'Return'});await sleep(80);wc.sendInputEvent({type:'keyUp',keyCode:'Return'});await sleep(1000);
   await check('session started',"document.querySelector('#output').value.includes('GameTask: Game setup complete')");
   await check('WebGL valid',"Module.canvas.getContext('webgl').getError()===0");
   fs.mkdirSync(output,{recursive:true});fs.writeFileSync(path.join(output,'gameplay.png'),(await wc.capturePage()).toPNG());
  }
  finish(0);
 }catch(e){report.error=String(e);finish(1)}
});
