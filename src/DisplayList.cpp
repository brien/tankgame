#ifdef _WIN32
// If building in windows:
#pragma warning(disable : 4996)
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#elif __APPLE__
// If building on macOS:
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
// If building on Linux:
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "DisplayList.h"

DisplayList::DisplayList(int num)
{
    enumIdx = 0;
    this->num = num;
    // Only call glGenLists if num > 0 and OpenGL context exists
    if (num > 0) {
        idx = glGenLists(num);
    } else {
        idx = 0;
    }
}

void DisplayList::BeginNewList()
{
    ResetList();
    NewList();
}

void DisplayList::NextNewList()
{
    glEndList();
    enumIdx++;
    if (enumIdx >= idx + num || enumIdx < idx)
    {
        // Can't create new list. Index out of bound.
        return;
    }
    glNewList(enumIdx, GL_COMPILE);
}

void DisplayList::EndNewList()
{
    glEndList();
}

void DisplayList::ResetList()
{
    enumIdx = idx;
}

void DisplayList::NewList()
{
    glNewList(enumIdx, GL_COMPILE);
}

void DisplayList::EndList()
{
    glEndList();
    enumIdx++;
}

void DisplayList::Call(int i)
{
    glCallList(idx + i);
}

void DisplayList::Close()
{
    glDeleteLists(idx, num);
}