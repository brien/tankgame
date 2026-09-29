const { chromium } = require('playwright');
const fs = require('fs');
const out = __dirname;
(async () => {
  const browser = await chromium.launch({ headless: false });
  const page = await browser.newPage({ viewport: { width: 1280, height: 1000 } });
  const logs = [], errors = [], requests = [], probes = [];
  page.on('console', m => logs.push({ type: m.type(), text: m.text() }));
  page.on('pageerror', e => errors.push(e.stack));
  page.on('response', r => requests.push({ url: r.url(), status: r.status() }));
  page.on('requestfailed', r => errors.push(`${r.url()}: ${r.failure()?.errorText}`));
  const key = async (k, delay = 250) => {
    await page.locator('#canvas').focus();
    await page.keyboard.press(k, { delay });
    await page.waitForTimeout(300);
  };
  const shot = async name => {
    await page.locator('#canvas').screenshot({ path: `${out}/browser-${name}.png` });
    probes.push(await page.evaluate(name => {
      const c = document.querySelector('#canvas'), gl = c.getContext('webgl');
      return { name, error: gl.getError(), lost: gl.isContextLost(), width: c.width, height: c.height };
    }, name));
    console.log(name);
    if (errors.length) throw Error(errors.join('\n'));
  };
  let stopped = false;
  try {
    await page.goto('http://127.0.0.1:8766/tankgame-linux.html');
    await page.waitForFunction(() => document.querySelector('#output').value.includes('Browser first frame completed'));
    await shot('title'); await key('Enter'); await page.waitForTimeout(1200); await shot('idle');
    await key('d', 500); await key('w', 800); await shot('moved');
    await page.mouse.move(750, 400, { steps: 10 }); await key('ArrowRight', 500); await shot('turret');
    await key('Space', 150); await shot('jump');
    await page.mouse.down(); await page.waitForTimeout(700); await shot('firing'); await page.mouse.up();
    await key('Escape'); await shot('returned-menu'); await key('Enter'); await page.waitForTimeout(1000); await shot('restart');
    await key('Escape'); await key('ArrowRight'); await key('Enter'); await page.waitForTimeout(1000);
    for (const mode of ['coop', 'versus']) {
      if (mode === 'versus') { await key('Escape'); await key('ArrowDown'); await key('Enter'); await page.waitForTimeout(1000); }
      await shot(`${mode}-idle`);
      await key('d', 500); await key('w', 800); await shot(`${mode}-moved`);
      await key('ArrowUp', 800); await shot(`${mode}-camera`);
      await page.mouse.down(); await page.waitForTimeout(700); await shot(`${mode}-firing`);
      await page.waitForTimeout(2000); await shot(`${mode}-later`); await page.mouse.up();
    }
    await key('Escape'); await key('ArrowLeft'); await key('Enter'); await page.waitForTimeout(1000); await shot('single-after-split');
    await key('Escape'); await key('Escape');
    await page.waitForFunction(() => document.querySelector('#output').value.includes('Browser frame loop stopped'));
    const count = logs.length; await page.waitForTimeout(1500); stopped = count === logs.length;
  } finally {
    fs.writeFileSync(`${out}/browser-console.json`, JSON.stringify({ browser: browser.version(), logs, errors, requests, probes, stopped }, null, 2));
    await browser.close();
  }
  if (!stopped || errors.length || probes.some(p => p.error || p.lost)) throw Error('Browser smoke failed');
})().catch(e => { console.error(e); process.exitCode = 1; });
