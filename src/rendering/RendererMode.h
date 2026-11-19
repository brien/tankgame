#pragma once

// Modern rendering is the only supported backend. This temporary facade keeps
// remaining legacy branches unreachable until their staged deletion is complete.
// TANKGAME_RENDERER overrides are ignored.
class RendererMode
{
public:
    static bool IsModern();
    static const char* Name();
};
