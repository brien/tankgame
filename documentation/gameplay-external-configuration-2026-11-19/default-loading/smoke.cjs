const {app,BrowserWindow}=require('electron');
const fs=require('fs');
const path=require('path');
const checks=[];
const sleep=ms=>new Promise(r=>setTimeout(r,ms));
const cases=[['default','Loaded gameplay.json'],['missing','Could not load gameplay.json'],['invalid','Could not load gameplay.json'],['alternative','Loaded alternative.json'],['invalid-alternative','Could not load alternative.json']];
app.setPath('userData','/tmp/tankgame-default-config-profile');
app.whenReady().then(async()=>{
 const timer=setTimeout(()=>app.exit(2),90000);
 const result={checks};
 try{
  for(const [name,message] of cases){
   const win=new BrowserWindow({width:1000,height:750});
   const wc=win.webContents;
   await win.loadURL('http://127.0.0.1:8767/'+name+'.html');
   for(let i=0;i<300;i++){
    if(await wc.executeJavaScript("document.querySelector('#output').value.includes('Browser first frame completed')"))break;
    await sleep(100);
   }
   const log=await wc.executeJavaScript("document.querySelector('#output').value");
   function check(label,value){checks.push({label:name+': '+label,value});if(!value)throw Error(name+': '+label)}
   check('provider diagnostic',log.includes(message));
   check('first frame',log.includes('Browser first frame completed'));
   check('fallback policy',log.includes('using built-in defaults')===message.startsWith('Could not'));
   check('no retry error',await wc.executeJavaScript("document.querySelector('#retry').hidden"));
   win.focus();wc.focus();await wc.executeJavaScript('Module.canvas.focus();void 0');await sleep(100);
   wc.sendInputEvent({type:'keyDown',keyCode:'Return'});await sleep(100);wc.sendInputEvent({type:'keyUp',keyCode:'Return'});await sleep(600);
   check('gameplay starts',await wc.executeJavaScript("document.querySelector('#output').value.includes('GameTask: Game setup complete')"));
   win.destroy();
  }
 }catch(e){result.error=String(e)}
 clearTimeout(timer);
 fs.writeFileSync(path.join(__dirname,'results.json'),JSON.stringify(result,null,2));
 app.exit(result.error?1:0);
});
// Keep the process alive while moving between the five test windows.
app.on('window-all-closed',()=>{});
