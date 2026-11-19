# Default gameplay file loading and fallback

The user requested automatic `gameplay.json` loading and logged fallback to
built-ins on any loading failure, retaining `--gameplay-config` for alternatives.
This supersedes the earlier opt-in/fatal-startup policy.

- Native and Emscripten builds passed.
- [249/249 native tests](ctest.txt) passed with graphics enabled, software OpenGL
  and dummy audio. New startup tests verify the provider diagnostic and that
  initialization continues to SDL after missing/invalid file fallback.
- [25/25 browser checks](results.json) passed across five cases: shipped default,
  missing default, invalid default, valid alternative, invalid alternative.
  Each verifies the log, first frame, fallback policy, absence of the error/retry
  screen, and entry into gameplay.
- `git diff --check` passed.

The browser fixtures use temporary HTML wrappers and an Emscripten
`onRuntimeInitialized` hook to edit/remove virtual files before `main()`. They
leave shipped source files and the generated browser artifacts unchanged.
Reproduce after building the browser target with its default configuration:

```sh
python3 documentation/gameplay-external-configuration-2026-11-19/default-loading/prepare.py runtime /tmp/gameplay-default-site
python3 -m http.server 8767 --directory /tmp/gameplay-default-site
```

Copy `smoke.cjs` to a temporary output directory, then run that copy with an
installed Electron executable and `ELECTRON_RUN_AS_NODE` unset. It records
`results.json` alongside itself. Run browser drivers sequentially because they
focus windows and inject keyboard input. This is automated Linux validation;
Windows/macOS and physical-controller qualification are not inferred.
