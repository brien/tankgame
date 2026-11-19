#include "rendering/RendererMode.h"

#include <cstring>
#include <iostream>

// CTest launches a fresh process for each environment to verify that old
// overrides cannot reactivate the retired compatibility backend.
int main(int argc, char** argv)
{
    if (argc != 2) return 2;
    const bool expectedModern = std::strcmp(argv[1], "modern") == 0;
    if (RendererMode::IsModern() != expectedModern ||
        std::strcmp(RendererMode::Name(), argv[1]) != 0)
    {
        std::cerr << "Expected " << argv[1] << ", got " << RendererMode::Name() << '\n';
        return 1;
    }
    return 0;
}
