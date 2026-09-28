#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class PixelFormat { RGB8, RGBA8 };

struct ImageData
{
    int width = 0;
    int height = 0;
    PixelFormat format = PixelFormat::RGB8;
    std::vector<std::uint8_t> pixels;

    int Channels() const { return format == PixelFormat::RGBA8 ? 4 : 3; }
    bool IsValid() const;
};

struct TextureUploadOptions
{
    bool repeat = false;
    bool mipmaps = true;
};

struct PortableTextureSettings
{
    bool repeat = false;
    bool mipmaps = false;
};

bool IsPowerOfTwo(int value);
PortableTextureSettings ChoosePortableTextureSettings(const ImageData& image,
                                                       TextureUploadOptions requested);
ImageData DecodeTga(const std::string& fileName);
