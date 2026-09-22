#pragma once

#include <memory>

#include "Geometry.h"

// A renderer resource facade.  The desktop backend currently stores an OpenGL
// display list behind Implementation; Emscripten stores an interleaved VBO and
// draws it with a small WebGL 1 shader. No OpenGL handle or type is exposed.
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
    // Owns the CPU geometry and prepares the platform renderer resource.
    void SetGeometry(const Geometry& geometry);
    // Explicit WebGL draw state boundary. These are no-ops for the native
    // display-list backend, whose callers continue to use fixed-function state.
    void SetMvpMatrix(const float* columnMajorMatrix);
    void SetDefaultColor(float red, float green, float blue, float alpha = 1.0f);
    void Call(int i);
    void Close();

private:
    class Implementation;
    std::shared_ptr<Implementation> implementation;
};
