#pragma once

#include <memory>
#include "ImageData.h"

class GpuTexture
{
public:
    explicit GpuTexture(const ImageData& image, TextureUploadOptions options = {});
    ~GpuTexture();
    GpuTexture(const GpuTexture&) = delete;
    GpuTexture& operator=(const GpuTexture&) = delete;
    void Bind(unsigned int unit = 0) const;
    unsigned int CompatibilityHandle() const;

private:
    class Implementation;
    std::unique_ptr<Implementation> implementation;
};
