// Usage: NODE_PATH=/path/to/playwright/node_modules node tools/browser-performance.cjs LABEL
// Serves no content itself: serve runtime/ on port 8765 first.
const { chromium } = require('playwright');
const fs = require('node:fs');
const { performance } = require('node:perf_hooks');
(async () => {
    const label = process.argv[2] || 'sample';
    const browser = await chromium.launch({ headless: false });
    try {
        const page = await browser.newPage({ viewport: { width: 1280, height: 1000 } });
        const logs = [], errors = [];
        page.on('console', msg => logs.push({ time: performance.now(), type: msg.type(), text: msg.text() }));
        page.on('pageerror', error => errors.push(String(error)));
        await page.goto(process.argv[3] || 'http://127.0.0.1:8765/tankgame-linux.html');
        await page.waitForFunction(() => document.querySelector('#output').value.includes('Browser first frame completed'));
        await page.locator('#canvas').focus();
        await page.keyboard.press('Enter', { delay: 250 });
        await page.waitForFunction(() => document.querySelector('#output').value.includes('GameTask: Game setup complete'));
        await page.waitForTimeout(3000);
        const gpu = await page.evaluate(() => {
            const canvas = document.querySelector('#canvas');
            const gl = canvas.getContext('webgl2') || canvas.getContext('webgl');
            const ext = gl.getExtension('WEBGL_debug_renderer_info');
            return { renderer: ext && gl.getParameter(ext.UNMASKED_RENDERER_WEBGL), width: canvas.width, height: canvas.height, visibility: document.visibilityState };
        });
        const samples = [];
        const glErrors = [];
        for (const state of ['idle', 'firing']) {
            if (state === 'firing') {
                await page.mouse.move(650, 400);
                await page.mouse.down();
                await page.waitForTimeout(3000);
            }
            const start = performance.now();
            await page.waitForTimeout(20000);
            const end = performance.now();
            const intervalLogs = logs.filter(log => log.time >= start && log.time <= end);
            // Skip the first report because its interval straddles sample start.
            const reports = intervalLogs.filter(log => log.text.startsWith('Browser perf:')).slice(1).map(log =>
                Object.fromEntries([...log.text.matchAll(/(\w+)=([\d.]+)/g)].map(m => [m[1], Number(m[2])])));
            const frames = reports.reduce((sum, r) => sum + r.frames, 0);
            const elapsed = reports.reduce((sum, r) => sum + r.interval_ms, 0);
            const result = { state, start, end, stdoutLines: intervalLogs.filter(log => log.type === 'log').length, seconds: (end - start) / 1000, lines: intervalLogs.length,
                linesPerSecond: intervalLogs.length * 1000 / (end - start), frames, measuredMs: elapsed,
                fps: frames * 1000 / elapsed, avgMs: elapsed / frames,
                tickMs: reports.reduce((sum, r) => sum + r.tick_ms * r.frames, 0) / frames,
                reports };
            samples.push(result);
            fs.writeFileSync(`/tmp/tankgame-${label}.json`, JSON.stringify({ browser: browser.version(), gpu, samples, errors }, null, 2));
            fs.writeFileSync(`/tmp/tankgame-${label}-console.json`, JSON.stringify(logs));
            console.log(JSON.stringify(result));
            // Probe outside measured intervals; never drain errors every frame.
            glErrors.push(await page.evaluate(state => {
                const canvas = document.querySelector('#canvas');
                const gl = canvas.getContext('webgl2') || canvas.getContext('webgl');
                return { state, error: gl.getError(), contextLost: gl.isContextLost() };
            }, state));
            await page.screenshot({ path: `/tmp/tankgame-${label}-${state}.png` });
        }
        await page.mouse.up();
        await page.mouse.move(850, 400, { steps: 20 });
        await page.waitForTimeout(500);
        await page.screenshot({ path: `/tmp/tankgame-${label}-rotated.png` });
        // Existing gameplay flow: Escape returns to MENU; a second press exits.
        await page.keyboard.press('Escape', { delay: 2500 });
        await page.waitForTimeout(500);
        await page.keyboard.press('Escape', { delay: 2500 });
        await page.waitForFunction(() => document.querySelector('#output').value.includes('Browser frame loop stopped'));
        const countAtStop = logs.filter(log => log.text.startsWith('Browser perf:')).length;
        await page.waitForTimeout(1500);
        const stopped = countAtStop === logs.filter(log => log.text.startsWith('Browser perf:')).length;
        const webglDiagnostics = logs.filter(log => /WebGL:|OpenGL Error|context lost|shader compilation failed|program link failed/i.test(log.text));
        const result = { browser: browser.version(), gpu, samples, errors, glErrors, webglDiagnostics, stopped };
        fs.writeFileSync(`/tmp/tankgame-${label}.json`, JSON.stringify(result, null, 2));
        fs.writeFileSync(`/tmp/tankgame-${label}-console.json`, JSON.stringify(logs));
        console.log(JSON.stringify({ browser: result.browser, gpu, errors, glErrors, webglDiagnostics, stopped }));
        if (!stopped || errors.length || webglDiagnostics.length ||
            glErrors.some(probe => probe.error !== 0 || probe.contextLost) ||
            samples.some(sample => sample.frames === 0)) {
            throw new Error('Browser validation failed; see saved results and console events.');
        }
    } finally { await browser.close(); }
})().catch(error => { console.error(error); process.exitCode = 1; });
