
#ifdef _WIN32
// If building in windows:
#pragma warning(disable : 4996)
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#else
//if not:
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

#include "DisplayList.h"

DisplayList::DisplayList(int num)
{
    enumIdx = 0;
    this->num = num;
    idx = glGenLists(num);
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
        //Can't create new list. Index out of bound.
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