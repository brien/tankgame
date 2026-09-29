#pragma once

#include <memory>

#include "BasicMaterial.h"

class GpuGeometry;

// Owns the one simple program used by the migrated modern slice and performs
// explicit material/transform draw submission for every compatible geometry.
class ModernRenderer
{
public:
    ModernRenderer();
    ~ModernRenderer();
    ModernRenderer(const ModernRenderer&) = delete;
    ModernRenderer& operator=(const ModernRenderer&) = delete;

    void Draw(const GpuGeometry& geometry, const float* columnMajorMvp,
              const BasicMaterial& material) const;
    const void* ProgramIdentity() const;

private:
    class ShaderProgram;
    std::unique_ptr<ShaderProgram> program;
};
