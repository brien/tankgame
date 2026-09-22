#pragma once

#include <memory>

// A renderer resource facade.  The desktop backend currently stores an OpenGL
// display list behind Implementation; no OpenGL handle or type is exposed to
// callers.  A future renderer can replace Implementation with buffered
// geometry without changing draw sites.
class DisplayList
{
public:
    explicit DisplayList(int num = 0);

    void BeginNewList();
    void NextNewList();
    void EndNewList();
    void ResetList();
    void NewList();
    void EndList();
    void Call(int i);
    void Close();

private:
    class Implementation;
    std::shared_ptr<Implementation> implementation;
};
