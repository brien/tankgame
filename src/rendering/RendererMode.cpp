#include "RendererMode.h"

bool RendererMode::IsModern()
{
    // The compatibility backend has been retired. Old environment overrides
    // are intentionally ignored, including TANKGAME_RENDERER=compatibility.
    return true;
}

const char* RendererMode::Name()
{
    return "modern";
}
