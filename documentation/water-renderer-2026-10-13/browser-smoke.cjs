// Run as an Electron main script with a standalone Electron executable.
// Serve runtime/ on port 8766 first. Uses real WebGL and captures the canvas.
const {app, BrowserWindow}=require('electron');
const fs=require('fs');
const path=require('path');
app.setPath('userData', '/tmp/tankgame-terrain-browser-profile');
const out=__dirname;
const report={versions:process.versions,errors:[],logs:[],probes:[],stopped:false};
const sleep=ms=>new Promise(r=>setTimeout(r,ms));
let stage='startup';
const mark=s=>{stage=s;console.log(s);};
const watchdog=setTimeout(()=>{
 report.errors.push('Harness timed out at '+stage);
 fs.writeFileSync(path.join(out,'browser-smoke.json'),JSON.stringify(report,null,2));
 app.exit(1);
},60000);
app.whenReady().then(async()=>{
 mark('ready');
 const win=new BrowserWindow({width:1320,height:1050,webPreferences:{backgroundThrottling:false}});
 const wc=win.webContents;
 wc.on('console-message',(_e,details,message)=>report.logs.push(typeof details==='object'?details:{level:details,message}));
 wc.on('render-process-gone',(_e,detail)=>report.errors.push(detail));
 const evaluate=script=>wc.executeJavaScript(script);
 const wait=async(script,ms=30000)=>{const end=Date.now()+ms;while(Date.now()<end){if(await evaluate(script))return;await sleep(100);}throw Error('Timed out: '+script);};
 const key=async(keyCode,ms=200)=>{wc.sendInputEvent({type:'keyDown',keyCode});await sleep(ms);wc.sendInputEvent({type:'keyUp',keyCode});await sleep(300);};
 const probe=async(name)=>{
  mark('probe '+name);
  const p=await evaluate(`(()=>{const c=document.querySelector('#canvas');const g=c.getContext('webgl');const e=g.getExtension('WEBGL_debug_renderer_info');return {name:${JSON.stringify(name)},error:g.getError(),lost:g.isContextLost(),renderer:e&&g.getParameter(e.UNMASKED_RENDERER_WEBGL),counts:{...terrainSmoke.counts},uploads:terrainSmoke.uploads,waterDraws:terrainSmoke.waterDraws,waterStates:terrainSmoke.waterStates,samples:terrainSmoke.samples,rect:{x:c.getBoundingClientRect().x,y:c.getBoundingClientRect().y,width:c.width,height:c.height}}})()`);
  report.probes.push(p);
  const im=await wc.capturePage(p.rect);fs.writeFileSync(path.join(out,`browser-${name}.png`),im.toPNG());
  if(p.error||p.lost)throw Error('WebGL probe failed: '+name);
 };
 try{
  mark('create renderer');await win.loadURL('about:blank');
  mark('debugger');wc.debugger.attach('1.3');await wc.debugger.sendCommand('Page.enable');
  mark('inject');
  await wc.debugger.sendCommand('Page.addScriptToEvaluateOnNewDocument',{source:`
    window.terrainSmoke={counts:{},samples:[],uploads:0,waterDraws:0,waterStates:[]};
    window.addEventListener('error',e=>console.error('PAGE_ERROR '+e.message));
    const draw=WebGLRenderingContext.prototype.drawArrays;
    WebGLRenderingContext.prototype.drawArrays=function(mode,first,count){
      const s=window.terrainSmoke;s.counts[count]=(s.counts[count]||0)+1;
      if(count>100&&s.samples.length<20)s.samples.push({count,depth:this.isEnabled(this.DEPTH_TEST),write:this.getParameter(this.DEPTH_WRITEMASK),blend:this.isEnabled(this.BLEND),front:this.getParameter(this.FRONT_FACE)});
      if(this.isEnabled(this.POLYGON_OFFSET_FILL)) {
        ++s.waterDraws;
        if(s.waterStates.length<8)s.waterStates.push({depth:this.isEnabled(this.DEPTH_TEST),write:this.getParameter(this.DEPTH_WRITEMASK),blend:this.isEnabled(this.BLEND),cull:this.isEnabled(this.CULL_FACE),src:this.getParameter(this.BLEND_SRC_RGB),dst:this.getParameter(this.BLEND_DST_RGB),offset:this.getParameter(this.POLYGON_OFFSET_FACTOR)});
      }
      return draw.call(this,mode,first,count);
    };
    const upload=WebGLRenderingContext.prototype.bufferData;
    WebGLRenderingContext.prototype.bufferData=function(...args){++terrainSmoke.uploads;return upload.apply(this,args);};
  `});
  mark('load');await win.loadURL('http://127.0.0.1:8766/tankgame-linux.html');
  mark('loaded');win.focus();wc.focus();
  await wait("document.querySelector('#output').value.includes('Browser first frame completed')");
  await probe('title');
  await evaluate("document.querySelector('#canvas').focus()");
  await key('Return');
  await wait("document.querySelector('#output').value.includes('GameTask::StartGame') || terrainSmoke.counts[36]>0");
  await sleep(1500);await probe('idle');
  await sleep(2000);await probe('idle-cached');
  await key('w',650);await probe('moved');
  const rect=report.probes.at(-1).rect;
  wc.sendInputEvent({type:'mouseMove',x:Math.round(rect.x+850),y:Math.round(rect.y+380)});
  wc.sendInputEvent({type:'mouseDown',button:'left',clickCount:1,x:Math.round(rect.x+850),y:Math.round(rect.y+380)});
  await sleep(10000);await probe('firing');
  wc.sendInputEvent({type:'mouseUp',button:'left',clickCount:1,x:Math.round(rect.x+850),y:Math.round(rect.y+380)});
  await key('Escape');await key('Return');await sleep(1000);await probe('restart');
  await key('Escape');await key('Escape');
  await wait("document.querySelector('#output').value.includes('Browser frame loop stopped')",5000);
  report.stopped=true;
  const errors=report.logs.filter(x=>/PAGE_ERROR|WebGL.*(INVALID|ERROR)|shader.*fail|memory access out of bounds|RuntimeError/i.test(x.message));
  if(errors.length)throw Error(JSON.stringify(errors));
  if(!report.probes.some(p=>p.waterDraws>0))throw Error('No water draws observed');
  if(report.probes.some(p=>p.waterStates.some(s=>!s.depth||s.write||!s.blend||s.cull||s.src!==770||s.dst!==1||s.offset!==-1)))throw Error('Incorrect water state');
  if(report.probes[1].uploads!==report.probes[2].uploads)throw Error('Unchanged terrain reuploaded');
 }catch(e){report.errors.push(e.stack);process.exitCode=1;}
 finally{
  fs.writeFileSync(path.join(out,'browser-smoke.json'),JSON.stringify(report,null,2));
  console.log(JSON.stringify({stopped:report.stopped,errors:report.errors,probes:report.probes.map(({name,error,lost,uploads,renderer})=>({name,error,lost,uploads,renderer}))}));
  clearTimeout(watchdog);win.destroy();app.exit(report.errors.length?1:0);
 }
});
