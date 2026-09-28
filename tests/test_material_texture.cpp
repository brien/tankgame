#include <gtest/gtest.h>

#include "rendering/BasicMaterial.h"
#include "rendering/ImageData.h"

TEST(BasicMaterialTest, DefaultsToOpaqueWhiteWithoutTexture)
{
    const BasicMaterial material;
    EXPECT_FLOAT_EQ(material.red, 1.0f);
    EXPECT_FLOAT_EQ(material.green, 1.0f);
    EXPECT_FLOAT_EQ(material.blue, 1.0f);
    EXPECT_FLOAT_EQ(material.alpha, 1.0f);
    EXPECT_FALSE(material.HasTexture());
}

TEST(TextureSettingsTest, PotImagesPreserveRequestedMipmapsAndRepeat)
{
    ImageData image{128, 64, PixelFormat::RGB8, std::vector<std::uint8_t>(128 * 64 * 3)};
    const auto settings = ChoosePortableTextureSettings(image, {true, true});
    EXPECT_TRUE(settings.repeat);
    EXPECT_TRUE(settings.mipmaps);
    EXPECT_TRUE(image.IsValid());
}

TEST(TextureSettingsTest, NpotImagesUseWebGlOneSafeSampling)
{
    ImageData image{63, 32, PixelFormat::RGBA8, std::vector<std::uint8_t>(63 * 32 * 4)};
    const auto settings = ChoosePortableTextureSettings(image, {true, true});
    EXPECT_FALSE(settings.repeat);
    EXPECT_FALSE(settings.mipmaps);
}
