#pragma once

#include <array>
#include <memory>

#include "rendering/GpuTexture.h"
#include "rendering/ImageData.h"

enum TextureNames
{
    TEXTURE_ZERO, TEXTURE_ONE, TEXTURE_TWO, TEXTURE_THREE, TEXTURE_FOUR,
    TEXTURE_FIVE, TEXTURE_SIX, TEXTURE_SEVEN, TEXTURE_EIGHT, TEXTURE_NINE,
    TEXTURE_WHITE_CUBE, TEXTURE_BLACK_CUBE, TEXTURE_EXIT, TEXTURE_BANG,
    TEXTURE_X, TEXTURE_CHECKER, TEXTURE_HEART, TEXTURE_DIAMOND, TEXTURE_P,
    TEXTURE_STAR, TEXTURE_RING, TEXTURE_LONGSHOT, TEXTURE_BANKSHOT,
    TEXTURE_MULTISHOT, TEXTURE_SCORE, TEXTURE_ENEMY, TEXTURE_NAMES_COUNT
};

// Typed catalogue of shared texture and decoded image resources.
class TextureHandler
{
public:
    TextureHandler() = default;
    ~TextureHandler() = default;
    void LoadTextures();
    std::shared_ptr<const GpuTexture> GetTexture(TextureNames name) const;
    std::shared_ptr<const ImageData> GetImage(TextureNames name) const;

private:
    std::array<std::shared_ptr<GpuTexture>, TEXTURE_NAMES_COUNT> textures;
    std::array<std::shared_ptr<const ImageData>, TEXTURE_NAMES_COUNT> images;
    void LoadTexture(const char* fileName, TextureNames id, bool wrap);
};
