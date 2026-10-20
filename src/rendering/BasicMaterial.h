#pragma once

#include <memory>

class GpuTexture;

struct BasicMaterial
{
    // Overlays and other emissive surfaces remain unlit unless explicitly selected.
    bool lit = false;
    float red = 1.0f;
    float green = 1.0f;
    float blue = 1.0f;
    float alpha = 1.0f;
    std::shared_ptr<const GpuTexture> texture;

    // Screen-space stipple for the unit targeting segment. Four exact nibbles
    // avoid relying on 16-bit integer precision in GLES2 fragment shaders.
    bool targeting = false;
    float stippleFrame[4] = {}; // window origin xy, major-axis direction xy
    float stippleBits[4] = {};

    bool HasTexture() const { return static_cast<bool>(texture); }
};
