#include "TextureHandler.h"

#include "Logger.h"
#include "rendering/ImageData.h"

TextureHandler::TextureHandler() { compatibilityHandles.fill(0); }

void TextureHandler::LoadTexture(const char* fileName, TextureNames id, bool wrap)
{
    try {
        images[id] = std::make_shared<const ImageData>(DecodeTga(fileName));
        textures[id] = std::make_shared<GpuTexture>(*images[id],
                                                    TextureUploadOptions{wrap, true});
        compatibilityHandles[id] = textures[id]->CompatibilityHandle();
    } catch (const std::exception& error) {
        Logger::Get().Write("TextureHandler: %s\n", error.what());
    }
}

std::shared_ptr<const ImageData> TextureHandler::GetImage(TextureNames name) const
{
    return images.at(static_cast<std::size_t>(name));
}

std::shared_ptr<const GpuTexture> TextureHandler::GetTexture(TextureNames name) const
{
    return textures.at(static_cast<std::size_t>(name));
}

void TextureHandler::LoadTextures()
{
    static const char* files[TEXTURE_NAMES_COUNT] = {
        "0.tga", "1.tga", "2.tga", "3.tga", "4.tga", "5.tga", "6.tga",
        "7.tga", "8.tga", "9.tga", "cube1.tga", "cube2.tga", "trail.tga",
        "bang.tga", "x.tga", "cube12.tga", "heart.tga", "p_itemstar.tga",
        "p.tga", "star.tga", "ring.tga", "long.tga", "bank.tga", "multi.tga",
        "score.tga", "enemy.tga"};
    for (int i = 0; i < TEXTURE_NAMES_COUNT; ++i) {
        const std::string path = std::string("texture/") + files[i];
        LoadTexture(path.c_str(), static_cast<TextureNames>(i), i != TEXTURE_EXIT);
    }
}
