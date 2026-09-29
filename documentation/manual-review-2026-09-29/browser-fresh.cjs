const { chromium } = require('playwright');
const fs = require('fs');
(async () => {
  const browser = await chromium.launch({ headless: false });
  try {
    for (const mode of ['coop', 'versus']) {
      const page = await browser.newPage({ viewport: { width: 1280, height: 1000 } });
      const logs = [], errors = [], probes = [];
      page.on('console', m => logs.push({ type: m.type(), text: m.text() }));
      page.on('pageerror', e => errors.push(e.stack));
      const key = async (k, delay = 200) => {
        await page.locator('#canvas').focus();
        await page.keyboard.press(k, { delay }); await page.waitForTimeout(200);
      };
      const shot = async name => {
        await page.locator('#canvas').screenshot({ path: `${__dirname}/browser-fresh-${mode}-${name}.png` });
        console.log(mode, name);
        if (errors.length) throw Error(errors.join('\n'));
      };
      let stopped = false;
      try {
        await page.goto('http://127.0.0.1:8766/tankgame-linux.html');
        await page.waitForFunction(() => document.querySelector('#output').value.includes('Browser first frame completed'));
        probes.push(await page.evaluate(() => {
          const gl = document.querySelector('#canvas').getContext('webgl');
          const ext = gl.getExtension('WEBGL_debug_renderer_info');
          return { renderer: ext && gl.getParameter(ext.UNMASKED_RENDERER_WEBGL), version: gl.getParameter(gl.VERSION) };
        }));
        await key('ArrowRight'); if (mode === 'versus') await key('ArrowDown');
        await key('Enter'); await page.waitForTimeout(1000); await shot('idle');
        await key('d', 400); await key('w', 500); await shot('moved');
        await key('ArrowUp', 700); await shot('camera');
        await page.mouse.move(650, 400); await page.mouse.down();
        await page.waitForTimeout(700); await shot('firing');
        await page.waitForTimeout(2000); await shot('later'); await page.mouse.up();
        await key('Escape'); await key('ArrowLeft'); await key('Enter'); await page.waitForTimeout(1000); await shot('single-after-split');
        probes.push(await page.evaluate(() => {
          const gl = document.querySelector('#canvas').getContext('webgl');
          return { error: gl.getError(), lost: gl.isContextLost() };
        }));
        await key('Escape'); await key('Escape');
        await page.waitForFunction(() => document.querySelector('#output').value.includes('Browser frame loop stopped'), {}, { timeout: 5000 });
        stopped = true;
      } catch (e) { errors.push(`Harness: ${e.stack}`); console.error(mode, e.message); }
      finally {
        fs.writeFileSync(`${__dirname}/browser-fresh-${mode}.json`, JSON.stringify({ browser: browser.version(), logs, errors, probes, stopped }, null, 2));
        await page.close();
      }
    }
  } finally { await browser.close(); }
})().catch(e => { console.error(e); process.exitCode = 1; });
