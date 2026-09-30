# Desktop portability of the shared modern renderer

**Status date:** 2026-09-29

## Scope and previous assumptions

This milestone changes only the existing modern player/material slice. It does
not migrate terrain, enemies, effects, HUD/menu, text, lighting, audio, browser
UX, or multiplayer. Previously Linux enabled `GL_GLEXT_PROTOTYPES` and called
shader, buffer, attribute, active-texture, and mipmap functions as link-time
symbols. That happened to work with Linux GL headers and drivers, but Windows'
system OpenGL ABI exposes only 1.1 and macOS core contexts reject the GLSL 1.20
dialect. SDL context creation also requested desktop 2.1 without stating a
profile.

## Modern GL inventory and capability contract

The shared slice uses these groups:

| Classification | Calls |
| --- | --- |
| Desktop OpenGL 1.1 exports (called directly) | `glGenTextures`, `glDeleteTextures`, `glBindTexture`, `glPixelStorei`, `glTexImage2D`, `glTexParameteri`, `glDrawArrays`, and common state calls such as viewport, clear, depth, cull and blend |
| Explicitly loaded on desktop | shader/program creation, compilation, linking and queries; all VBO calls; vertex-attribute enable/disable/pointers; uniform uploads; `glActiveTexture`; `glGenerateMipmap` (with `glGenerateMipmapEXT` fallback) |
| Compatibility-only, invalid in a core modern path | display lists, matrix stack, immediate mode, fixed-function lighting/material/texture enable, line stipple, and GLU projection calls; none is part of `ModernRenderer`, `GpuGeometry`, or `GpuTexture`, although deferred renderer categories elsewhere still contain them |
| WebGL 1 / GLES2 compatible | the loaded group above through normal GLES symbols, plus textures, draw arrays and common state; NPOT policy remains clamp/no-mipmap |

The minimum contract for Linux/Windows modern is an OpenGL 2.1 compatibility
context with the 2.0 shader API, 1.5 VBO API, multitexture, and framebuffer
object mipmap generation exposed as either the core or EXT spelling. The loader
checks every non-1.1 entry point before renderer resources are created and
returns one diagnostic listing missing names. macOS modern requests OpenGL 3.2
core, whose API exceeds that minimum but is the oldest programmable core
context offered by SDL on macOS. Emscripten requests GLES 2 / WebGL 1.

`GLFunctions` is the single proc-address boundary. Desktop obtains all modern
entry points from `SDL_GL_GetProcAddress`; Linux no longer asks headers for
extension prototypes, and Windows does not require modern declarations or
exports from `opengl32`. Emscripten wrappers call its linked GLES functions.

## Context and shader strategy

`GraphicsCapabilities` makes the request testable without a live GL context:

| Target/mode | SDL request | Shader dialect | Status/constraint |
| --- | --- | --- | --- |
| Linux legacy | 2.1 compatibility | none | preserved |
| Linux modern | 2.1 compatibility | GLSL 1.20 | coexists with deferred compatibility categories |
| Windows legacy | 2.1 compatibility | none | architectural/build-system path only |
| Windows modern | 2.1 compatibility | GLSL 1.20 | modern calls use SDL loading |
| macOS legacy | rejected with a clear message | none | deliberately unsupported; no core-safe legacy renderer is claimed |
| macOS modern | 3.2 core | GLSL 1.50 core | modern slice is structurally core compatible |
| Emscripten | ES 2.0 | GLSL ES 1.00 | normal WebGL/GLES symbols |

One shader builder retains the logical implementation and emits only dialect
syntax: `attribute`/`varying` and `texture2D` for GLSL 1.20/ES 1.00; `in`,
vertex `out`, fragment `in`, an explicit fragment output, and `texture` for
GLSL 1.50 core; and the ES precision qualifier only for WebGL.

The macOS context and shared modern slice are now architecturally prepared, but
the entire game is **not** claimed core-clean: the intentionally deferred
terrain/enemy/effect/HUD/menu paths still contain compatibility calls. Running
those paths in a macOS core context remains a later convergence task.

## Validation evidence

* Linux Release configure and compilation completed, and all 89 tests passed,
  including focused context/profile and three-dialect shader tests.
* Linux modern initialized and remained in its main loop for the five-second
  smoke interval, with no loader or shader error. Linux legacy was attempted
  but segfaulted during headless startup after reporting the missing
  `XDG_RUNTIME_DIR`; this environment has no X server or Xvfb. Neither smoke
  reached a visual gameplay fixture, so no new visual claim is made and the
  prior documented headed/Xvfb evidence remains the visual baseline.
* Emscripten Release configured, compiled, linked and packaged HTML, JavaScript,
  WASM and preload data. Browser execution was not available, so visuals were
  not revalidated.
* No Windows SDK/toolchain or Windows runtime was available. Windows received
  architectural and build-system inspection only; compile, link and runtime
  remain unverified.
* No macOS SDK/toolchain or macOS runtime was available. macOS received
  architectural and build-system inspection only; compile, link and runtime
  remain unverified.

## Desktop CI qualification gate

The repository previously had no GitHub Actions workflow (and no other CI
configuration); `.github/copilot-instructions.md` was guidance rather than an
executable workflow. Consequently there were no hosted platforms, compilers,
dependency-install steps, test runs, uploaded artifacts, or Emscripten CI. The
browser build evidence above remains local/manual evidence.

`desktop-ci.yml` now defines the deliberately small native matrix below. All
three jobs configure a Release Ninja build with `BUILD_TESTS=ON`, build the game
and `tankgame_tests`, and treat configure, compilation, linking, and CTest
failures as job failures. No build artifacts are uploaded: this is a
qualification gate, not a packaging workflow.

| Job | Toolchain and dependencies | Qualification supplied by a green job | Explicit non-claim |
| --- | --- | --- | --- |
| Linux | GitHub `ubuntu-latest`, GCC, APT `ninja-build`, Mesa GL/GLU, SDL2/mixer/ttf development packages, and packaged nlohmann-json | Compile validated; link validated; full tests validated | Runtime smoke is not part of this gate; prior documented runtime evidence remains separate |
| Windows | GitHub `windows-latest`, MinGW-w64 GCC/Ninja/CMake and SDL2/mixer/ttf plus nlohmann-json from MSYS2's MINGW64 repository | Compile validated; link validated; platform-independent tests validated. Compiling and linking the complete native target specifically includes `ModernRenderer`, `GpuGeometry`, `GpuTexture`, and `GLFunctions`, with `_WIN32` selecting the OpenGL 1.1 system-header boundary and SDL proc loading | Runtime renderer unvalidated; this is not MSVC qualification |
| macOS | GitHub `macos-latest`, AppleClang/Ninja, framework OpenGL, and SDL2/mixer/ttf plus nlohmann-json from Homebrew | Compile validated; link validated; platform-independent tests validated. The native target includes the shared modern renderer and the Apple platform-header branch; focused tests enforce the 3.2 core request and GLSL 1.50 core shader dialect | Runtime renderer unvalidated; compilation of retained compatibility sources does not make them legal in a core context, and legacy mode remains deliberately rejected on macOS |

The table describes the qualification produced **when the corresponding hosted
job is green**. At the time this workflow was authored, GitHub-hosted Windows
and macOS jobs could not be executed in the local Codex environment, so their
new compile/link/test status is pending the first pushed Actions run and is not
claimed as successful here. Linux equivalent configure/build/CTest commands
are locally reproducible; the Actions run remains the authoritative CI result.

The tests compile the production sources into `tankgame_tests`, so the desktop
jobs do not validate only isolated headers. Focused coverage includes graphics
context/profile policy and all shader dialects, neutral geometry preparation,
and renderer resource-catalogue ownership. GPU calls and real window/context
behavior are deliberately outside those headless unit tests. A candidate local
Xvfb legacy smoke exited with a pre-existing startup segmentation fault rather
than remaining alive for its bounded interval, so making the new matrix depend
on that fragile check would not be reliable. No desktop job therefore makes a
new synthetic runtime claim; the earlier Linux runtime evidence remains
documented above.

No Emscripten job was added. There was no existing browser CI to preserve, and
installing an SDK and packaging/browser stack would exceed this native
qualification milestone. Adding a narrow Emscripten build job remains a later
CI enhancement; the existing local build procedure and evidence are unchanged.

## Remaining validation and recommended next milestone

After the hosted matrix is green, real Windows and macOS machines should exercise
loader diagnostics and run the modern slice. Manual macOS work must also
confirm that startup does not reach any deferred compatibility category before
that category becomes core-safe. Linux should be smoked again under X11/Xvfb or
a real display, and the packaged browser should be opened in a WebGL-capable
browser.

The recommended next milestone is **real-machine runtime qualification of this
portability boundary on Windows and macOS**. Only after that evidence should a
separately scoped milestone convert one already-in-scope deferred draw category
to explicit modern submission. Do not broaden into terrain as a
whole, UI/text, lighting, audio, browser UX, or multiplayer in that qualification
work.
