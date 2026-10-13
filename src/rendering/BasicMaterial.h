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

    bool HasTexture() const { return static_cast<bool>(texture); }
};
