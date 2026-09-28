#pragma once

// Process-wide renderer selection. Emscripten always uses the programmable
// renderer. Desktop defaults to the compatibility renderer and accepts
// TANKGAME_RENDERER=modern for the convergence path.
class RendererMode
{
public:
    static bool IsModern();
    static const char* Name();
};
