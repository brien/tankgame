#pragma once

// Process-wide renderer selection. Emscripten always uses the programmable
// renderer. Desktop defaults to modern; TANKGAME_RENDERER=compatibility
// explicitly selects the fallback on Linux/Windows (unsupported on macOS).
class RendererMode
{
public:
    static bool IsModern();
    static const char* Name();
};
