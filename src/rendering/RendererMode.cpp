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
        // Compatibility is an explicit opt-in; unset, empty and unknown values
        // use the shared modern renderer.
        return value == nullptr || std::strcmp(value, "compatibility") != 0;
    }();
    return modern;
#endif
}

const char* RendererMode::Name()
{
    return IsModern() ? "modern" : "compatibility";
}
