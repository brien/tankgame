#pragma once

#include <memory>

#include "Geometry.h"

class GpuGeometry;

// A transitional resource facade. Modern mode stores CPU geometry plus an
// interleaved GPU geometry resource; native compatibility mode retains display
// lists temporarily. Programs and draw state are owned elsewhere.
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
    void Call(int i);
    const GpuGeometry& ModernGeometry() const;
    void Close();

private:
    class Implementation;
    std::shared_ptr<Implementation> implementation;
};
