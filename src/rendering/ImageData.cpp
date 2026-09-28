#include "ImageData.h"

#include <cstdio>
#include <algorithm>
#include <stdexcept>

namespace {
std::uint16_t Read16(FILE* file)
{
    unsigned char bytes[2];
    if (std::fread(bytes, 1, 2, file) != 2) throw std::runtime_error("truncated TGA");
    return static_cast<std::uint16_t>(bytes[0] | (bytes[1] << 8));
}
}

bool ImageData::IsValid() const
{
    return width > 0 && height > 0 &&
        pixels.size() == static_cast<std::size_t>(width * height * Channels());
}

bool IsPowerOfTwo(int value)
{
    return value > 0 && (value & (value - 1)) == 0;
}

PortableTextureSettings ChoosePortableTextureSettings(const ImageData& image,
                                                       TextureUploadOptions requested)
{
    const bool pot = IsPowerOfTwo(image.width) && IsPowerOfTwo(image.height);
    return {requested.repeat && pot, requested.mipmaps && pot};
}

ImageData DecodeTga(const std::string& fileName)
{
    FILE* file = std::fopen(fileName.c_str(), "rb");
    if (!file) throw std::runtime_error("failed to open TGA: " + fileName);
    try {
        const int idLength = std::fgetc(file);
        const int colorMapType = std::fgetc(file);
        const int imageType = std::fgetc(file);
        std::fseek(file, 9, SEEK_CUR);
        const int width = Read16(file);
        const int height = Read16(file);
        const int bits = std::fgetc(file);
        std::fgetc(file); // descriptor; preserve the legacy row order
        if (colorMapType != 0 || (imageType != 2 && imageType != 10) ||
            (bits != 16 && bits != 24 && bits != 32) || width <= 0 || height <= 0)
            throw std::runtime_error("unsupported TGA: " + fileName);
        std::fseek(file, idLength, SEEK_CUR);

        ImageData image;
        image.width = width;
        image.height = height;
        image.format = bits == 32 ? PixelFormat::RGBA8 : PixelFormat::RGB8;
        image.pixels.resize(static_cast<std::size_t>(width * height * image.Channels()));
        auto readPixel = [&]() {
            unsigned char source[4] = {};
            const int sourceChannels = bits / 8;
            if (std::fread(source, 1, sourceChannels, file) != static_cast<std::size_t>(sourceChannels))
                throw std::runtime_error("truncated TGA: " + fileName);
            unsigned char pixel[4] = {source[2], source[1], source[0], source[3]};
            if (bits == 16) {
                const std::uint16_t packed = source[0] | (source[1] << 8);
                pixel[0] = static_cast<unsigned char>(((packed >> 10) & 31) << 3);
                pixel[1] = static_cast<unsigned char>(((packed >> 5) & 31) << 3);
                pixel[2] = static_cast<unsigned char>((packed & 31) << 3);
            }
            return std::vector<unsigned char>(pixel, pixel + image.Channels());
        };
        int written = 0;
        const int total = width * height;
        while (written < total) {
            int count = 1;
            bool run = false;
            if (imageType == 10) {
                const int packet = std::fgetc(file);
                if (packet == EOF) throw std::runtime_error("truncated TGA packet");
                run = (packet & 0x80) != 0;
                count = (packet & 0x7f) + 1;
            }
            std::vector<unsigned char> repeated;
            if (run) repeated = readPixel();
            for (int i = 0; i < count && written < total; ++i, ++written) {
                const auto pixel = run ? repeated : readPixel();
                std::copy(pixel.begin(), pixel.end(), image.pixels.begin() + written * image.Channels());
            }
        }
        std::fclose(file);
        return image;
    } catch (...) {
        std::fclose(file);
        throw;
    }
}
