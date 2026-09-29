const { chromium } = require('playwright');
const fs = require('fs');
(async () => {
  const b = await chromium.launch({ headless: false });
  const p = await b.newPage({ viewport: { width: 1280, height: 1000 } });
  const samples = [], errors = [];
  p.on('pageerror', e => errors.push(e.stack));
  const key = async k => { await p.locator('#canvas').focus(); await p.keyboard.press(k, { delay: 150 }); await p.waitForTimeout(150); };
  const sample = async label => {
    // Read the viewport after the game has rendered, inside the next animation callback.
    samples.push(await p.evaluate(label => new Promise(resolve => requestAnimationFrame(() => {
      const gl = document.querySelector('#canvas').getContext('webgl');
      resolve({ label, viewport: Array.from(gl.getParameter(gl.VIEWPORT)), error: gl.getError() });
    })), label));
    await p.locator('#canvas').screenshot({ path: `${__dirname}/browser-transition-${label}.png` });
  };
  try {
    await p.goto('http://127.0.0.1:8766/tankgame-linux.html');
    await p.waitForFunction(() => document.querySelector('#output').value.includes('Browser first frame completed'));
    await key('Enter'); await sample('initial-single');
    await key('Escape'); await key('ArrowRight'); await key('Enter'); await sample('coop');
    await key('Escape'); await key('ArrowLeft'); await key('Enter'); await sample('selected-single');
    await key('Escape'); await key('Escape');
    await p.waitForFunction(() => document.querySelector('#output').value.includes('Browser frame loop stopped'));
  } finally {
    fs.writeFileSync(`${__dirname}/browser-transition.json`, JSON.stringify({ samples, errors }, null, 2));
    await b.close();
  }
})().catch(e => { console.error(e); process.exitCode = 1; });
