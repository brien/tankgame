#include "RendererMode.h"

#include <cstdlib>
#include <cstring>

bool RendererMode::IsModern()
{
#ifdef __EMSCRIPTEN__
    return true;
#else
    static const bool modern = [] {
        const char* value = std::getenv("TANKGAME_RENDERER");
        return value != nullptr && std::strcmp(value, "modern") == 0;
    }();
    return modern;
#endif
}

const char* RendererMode::Name()
{
    return IsModern() ? "modern" : "compatibility";
}
