const {chromium}=require('playwright');const fs=require('fs');
const label=process.argv[2]||'diagnostic', runs=+(process.argv[3]||3), seconds=+(process.argv[4]||60);
const out=__dirname+'/browser-fire';fs.mkdirSync(out,{recursive:true});
(async()=>{const b=await chromium.launch({headless:false});try{for(let run=1;run<=runs;run++){
const p=await b.newPage({viewport:{width:1280,height:1000}});p.setDefaultTimeout(120000);let start=Date.now(), fireStart=0;const logs=[],errors=[],memory=[];
p.on('console',m=>{const r={seconds:(Date.now()-start)/1000,type:m.type(),text:m.text()};logs.push(r);if(/ERROR:|runtime error:|AddressSanitizer|SUMMARY:|NextLevel|respawn/.test(r.text))console.log(r.text)});
p.on('pageerror',e=>{const r={seconds:(Date.now()-fireStart)/1000,stack:e.stack};errors.push(r);console.log(JSON.stringify({run,error:r}))});
let stopped=false,probe=null;
try {await p.goto(process.env.TANKGAME_TEST_URL||'http://127.0.0.1:8765/tankgame-linux.html');await p.waitForFunction(()=>document.querySelector('#output').value.includes('Browser first frame completed'));await p.locator('#canvas').focus();await p.keyboard.press('Enter',{delay:250});await p.waitForFunction(()=>document.querySelector('#output').value.includes('Game setup complete'));await p.waitForTimeout(1500 + (process.env.TANKGAME_VARY_START ? run * 37 : 0));await p.mouse.move(650,400);await p.mouse.down();fireStart=Date.now();
for(let sec=0;sec<seconds;sec++){await p.waitForTimeout(1000);if(errors.length)break;if(sec%5===0)memory.push(await p.evaluate(()=>({heapBytes:typeof HEAPU8!=='undefined'?HEAPU8.byteLength:null,jsHeap:performance.memory?.usedJSHeapSize,at:performance.now()})));if(sec%10===9)console.log(JSON.stringify({label,run,firingSeconds:sec+1,perf:logs.filter(l=>l.text.startsWith('Browser perf:')).slice(-1),memory:memory.slice(-1)}))}
await p.mouse.up();await p.locator('#canvas').screenshot({path:out+'/run-'+run+'.png'});probe=await p.evaluate(()=>{let g=document.querySelector('#canvas').getContext('webgl');return {error:g.getError(),lost:g.isContextLost()}});
if(!errors.length){await p.locator('#canvas').focus();await p.keyboard.press('Escape',{delay:250});await p.waitForTimeout(400);await p.keyboard.press('Escape',{delay:250});await p.waitForFunction(()=>document.querySelector('#output').value.includes('Browser frame loop stopped'),{},{timeout:5000});stopped=true;}
} catch(e){errors.push({harness:true,stack:e.stack});console.log(e.stack)}
fs.writeFileSync(out+'/run-'+run+'.json',JSON.stringify({label,run,seconds,browser:b.version(),logs,errors,memory,probe,stopped},null,2));console.log(JSON.stringify({label,run,errors:errors.length,stopped,probe}));await p.close();if(errors.length && process.env.TANKGAME_STOP_ON_ERROR)break;
}}finally{await b.close()}})().catch(e=>{console.error(e);process.exitCode=1});
