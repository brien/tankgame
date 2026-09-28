#pragma once

#include <memory>

class GpuTexture;

struct BasicMaterial
{
    float red = 1.0f;
    float green = 1.0f;
    float blue = 1.0f;
    float alpha = 1.0f;
    std::shared_ptr<const GpuTexture> texture;

    bool HasTexture() const { return static_cast<bool>(texture); }
};
