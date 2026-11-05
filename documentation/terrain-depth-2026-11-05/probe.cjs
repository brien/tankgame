const {app,BrowserWindow}=require('electron');const fs=require('fs');const path=require('path');
const sleep=ms=>new Promise(r=>setTimeout(r,ms));
const out=path.join(__dirname,process.env.QA_CASE||'deployed');fs.mkdirSync(out,{recursive:true});let win;
app.whenReady().then(async()=>{try{
win=new BrowserWindow({width:1600,height:1000,webPreferences:{backgroundThrottling:false}});const wc=win.webContents;const js=s=>wc.executeJavaScript(s);
await win.loadURL(process.env.QA_URL||'https://briensmithmartinez.com/tankgame/tankgame-linux.html');win.focus();wc.focus();
for(let i=0;i<300;++i){if(await js("document.querySelector('#output').value.includes('Browser first frame completed')"))break;await sleep(100)}
if(!await js("document.querySelector('#output').value.includes('Browser first frame completed')"))throw Error('Game startup timed out');
await js('Module.canvas.focus()');
async function key(k,ms=100){wc.sendInputEvent({type:'keyDown',keyCode:k});await sleep(ms);wc.sendInputEvent({type:'keyUp',keyCode:k});await sleep(200)}
await key('Return');await key('a',350);await key('w',500);await key('Space',500);
await js(`window.g=Module.canvas.getContext('webgl');window.precisionReport={depth:g.getParameter(g.DEPTH_BITS),medium:g.getShaderPrecisionFormat(g.VERTEX_SHADER,g.MEDIUM_FLOAT).precision,high:g.getShaderPrecisionFormat(g.VERTEX_SHADER,g.HIGH_FLOAT).precision,renderer:g.getParameter(g.getExtension('WEBGL_debug_renderer_info').UNMASKED_RENDERER_WEBGL)};void 0`);
console.log(await js('precisionReport'));await sleep(300);
fs.writeFileSync(path.join(out,'default.png'),(await wc.capturePage()).toPNG());
await js(`window.fb=g.createFramebuffer();g.bindFramebuffer(g.FRAMEBUFFER,fb);window.tex=g.createTexture();g.bindTexture(g.TEXTURE_2D,tex);g.texImage2D(g.TEXTURE_2D,0,g.RGBA,Module.canvas.width,Module.canvas.height,0,g.RGBA,g.UNSIGNED_BYTE,null);g.texParameteri(g.TEXTURE_2D,g.TEXTURE_MIN_FILTER,g.NEAREST);g.texParameteri(g.TEXTURE_2D,g.TEXTURE_MAG_FILTER,g.NEAREST);g.framebufferTexture2D(g.FRAMEBUFFER,g.COLOR_ATTACHMENT0,g.TEXTURE_2D,tex,0);window.depth=g.createRenderbuffer();g.bindRenderbuffer(g.RENDERBUFFER,depth);g.renderbufferStorage(g.RENDERBUFFER,g.DEPTH_COMPONENT16,Module.canvas.width,Module.canvas.height);g.framebufferRenderbuffer(g.FRAMEBUFFER,g.DEPTH_ATTACHMENT,g.RENDERBUFFER,depth);g.bindTexture(g.TEXTURE_2D,null);g.bindRenderbuffer(g.RENDERBUFFER,null);precisionReport.framebufferComplete=g.checkFramebufferStatus(g.FRAMEBUFFER)===g.FRAMEBUFFER_COMPLETE;void 0`);
await sleep(500);
const data=await js(`(()=>{const w=Module.canvas.width,h=Module.canvas.height,pixels=new Uint8Array(w*h*4);g.readPixels(0,0,w,h,g.RGBA,g.UNSIGNED_BYTE,pixels);const c=document.createElement('canvas');c.width=w;c.height=h;const ctx=c.getContext('2d'),im=ctx.createImageData(w,h);for(let y=0;y<h;++y)im.data.set(pixels.subarray((h-1-y)*w*4,(h-y)*w*4),y*w*4);ctx.putImageData(im,0,0);precisionReport.error=g.getError();return c.toDataURL().split(',')[1]})()`);
fs.writeFileSync(path.join(out,'depth16.png'),Buffer.from(data,'base64'));fs.writeFileSync(path.join(out,'probe.json'),JSON.stringify(await js('precisionReport'),null,2));
const report=await js('precisionReport');if(report.error||!report.framebufferComplete)throw Error('Invalid WebGL probe');app.exit(0);
}catch(e){console.error(e);app.exit(1)}});setTimeout(()=>app.exit(2),45000);
