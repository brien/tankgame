const {chromium}=require('playwright');
const fs=require('fs');
(async()=>{
  const browser=await chromium.launch({headless:false, executablePath:'/usr/bin/chromium',args:['--no-sandbox','--disable-dev-shm-usage','--use-angle=swiftshader','--enable-unsafe-swiftshader']});
  const page=await browser.newPage({viewport:{width:1280,height:1000}});
  const report={browser:browser.version(),logs:[],errors:[],probes:[],stopped:false};
  page.on('console',m=>report.logs.push({type:m.type(),text:m.text()}));
  page.on('pageerror',e=>report.errors.push(e.stack));
  await page.addInitScript(()=>{
    window.enemySmoke={counts:{},samples:[]};
    const draw=WebGLRenderingContext.prototype.drawArrays;
    WebGLRenderingContext.prototype.drawArrays=function(mode,first,count){
      const s=window.enemySmoke;s.counts[count]=(s.counts[count]||0)+1;
      if(s.samples.length<30 && (count===36||count===24)) s.samples.push({count,depth:this.isEnabled(this.DEPTH_TEST),write:this.getParameter(this.DEPTH_WRITEMASK),blend:this.isEnabled(this.BLEND),front:this.getParameter(this.FRONT_FACE)});
      return draw.call(this,mode,first,count);
    };
  });
  const key=async k=>{await page.locator('#canvas').focus();await page.keyboard.press(k,{delay:200});await page.waitForTimeout(300);};
  const probe=async name=>{
    await page.locator('#canvas').screenshot({path:`${__dirname}/browser-${name}.png`});
    report.probes.push(await page.evaluate(name=>{
      const gl=document.querySelector('#canvas').getContext('webgl');
      const ext=gl.getExtension('WEBGL_debug_renderer_info');
      return {name,error:gl.getError(),lost:gl.isContextLost(),renderer:ext&&gl.getParameter(ext.UNMASKED_RENDERER_WEBGL),counts:{...window.enemySmoke.counts},samples:window.enemySmoke.samples};
    },name));
  };
  try{
    await page.goto('http://127.0.0.1:8766/tankgame-linux.html');
    await page.waitForFunction(()=>document.querySelector('#output').value.includes('Browser first frame completed'),{},{timeout:30000});
    // SDL keyboard focus can lag startup under software rendering; use the
    // existing click-to-start input and require real enemy draws before captures.
    await page.locator('#canvas').click({position:{x:640,y:360},delay:400});
    await page.waitForFunction(()=>window.enemySmoke.counts[36]>0,{},{timeout:15000});
    await page.waitForTimeout(1000);await probe('idle');
    await page.waitForTimeout(3000);await probe('enemy-motion');
    await page.keyboard.down('w');await page.waitForTimeout(700);await page.keyboard.up('w');
    await page.mouse.move(850,420);await probe('moved');
    await page.mouse.down();await page.waitForTimeout(700);await page.mouse.up();await probe('firing');
    await key('Escape');await key('Escape');
    await page.waitForFunction(()=>document.querySelector('#output').value.includes('Browser frame loop stopped'),{},{timeout:5000});
    report.stopped=true;
  }catch(e){report.errors.push(e.stack);process.exitCode=1;}
  finally{fs.writeFileSync(`${__dirname}/browser-smoke.json`,JSON.stringify(report,null,2));await browser.close();}
  console.log(JSON.stringify({browser:report.browser,stopped:report.stopped,errors:report.errors,probes:report.probes.map(({name,error,lost,renderer,counts})=>({name,error,lost,renderer,counts}))}));
})();
