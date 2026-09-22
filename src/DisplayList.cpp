#include "DisplayList.h"

#include <cstdio>
#include <cstdlib>

#ifndef __EMSCRIPTEN__
#ifdef _WIN32
#pragma warning(disable : 4996)
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#elif __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif

class DisplayList::Implementation
{
public:
#ifdef __EMSCRIPTEN__
    explicit Implementation(int count) : count(count) {}

    int count;
#else
    explicit Implementation(int count)
        : count(count), first(count > 0 ? glGenLists(count) : 0), current(first)
    {
    }

    ~Implementation()
    {
        if (first != 0)
            glDeleteLists(first, count);
    }

    int count;
    GLuint first;
    GLuint current;
#endif
};

#ifdef __EMSCRIPTEN__
namespace
{
[[noreturn]] void UnsupportedDisplayListOperation(const char* operation)
{
    std::fprintf(stderr,
        "DisplayList::%s cannot run on WebGL: geometry must be converted to "
        "backend-neutral vertex data first.\n",
        operation);
    std::abort();
}
}
#endif

DisplayList::DisplayList(int num)
    : implementation(std::make_shared<Implementation>(num))
{
}

void DisplayList::BeginNewList()
{
    ResetList();
    NewList();
}

void DisplayList::NextNewList()
{
#ifdef __EMSCRIPTEN__
    UnsupportedDisplayListOperation("NextNewList");
#else
    glEndList();
    ++implementation->current;
    if (implementation->current >= implementation->first + implementation->count ||
        implementation->current < implementation->first)
        return;
    glNewList(implementation->current, GL_COMPILE);
#endif
}

void DisplayList::EndNewList()
{
#ifdef __EMSCRIPTEN__
    UnsupportedDisplayListOperation("EndNewList");
#else
    glEndList();
#endif
}

void DisplayList::ResetList()
{
#ifdef __EMSCRIPTEN__
    // Resetting the cursor does not claim that the resource can be rendered.
#else
    implementation->current = implementation->first;
#endif
}

void DisplayList::NewList()
{
#ifdef __EMSCRIPTEN__
    UnsupportedDisplayListOperation("NewList");
#else
    glNewList(implementation->current, GL_COMPILE);
#endif
}

void DisplayList::EndList()
{
#ifdef __EMSCRIPTEN__
    UnsupportedDisplayListOperation("EndList");
#else
    glEndList();
    ++implementation->current;
#endif
}

void DisplayList::Call(int i)
{
#ifdef __EMSCRIPTEN__
    (void)i;
    UnsupportedDisplayListOperation("Call");
#else
    glCallList(implementation->first + i);
#endif
}

void DisplayList::Close()
{
    implementation.reset();
}
