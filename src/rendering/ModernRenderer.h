#pragma once

#include <memory>
#include <cstdint>

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
              const BasicMaterial& material, const float* normalTransform, bool lighting) const;
    const void* ProgramIdentity() const;

    // Lifetime submission totals, measured at glDrawArrays. Lines count as calls
    // but not triangles; these are submitted primitives, not visible pixels.
    struct SubmissionStats {
        std::uint64_t drawCalls = 0;
        std::uint64_t triangles = 0;
    };
    SubmissionStats GetSubmissionStats() const { return submissionStats; }

private:
    mutable SubmissionStats submissionStats;
    class ShaderProgram;
    std::unique_ptr<ShaderProgram> program;
};
