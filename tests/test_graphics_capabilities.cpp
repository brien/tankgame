#include <gtest/gtest.h>

#include "rendering/GraphicsCapabilities.h"

TEST(GraphicsCapabilities, KeepsLegacyOffCoreProfiles)
{
    const ContextRequest linuxLegacy = ChooseContextRequest(GraphicsPlatform::Linux, false);
    const ContextRequest windowsLegacy = ChooseContextRequest(GraphicsPlatform::Windows, false);
    EXPECT_TRUE(linuxLegacy.supported);
    EXPECT_EQ(linuxLegacy.profile, GraphicsProfile::Compatibility21);
    EXPECT_EQ(windowsLegacy.profile, GraphicsProfile::Compatibility21);
}

TEST(GraphicsCapabilities, SelectsMacCoreOnlyForModernRenderer)
{
    const ContextRequest modern = ChooseContextRequest(GraphicsPlatform::MacOS, true);
    EXPECT_TRUE(modern.supported);
    EXPECT_EQ(modern.major, 3);
    EXPECT_EQ(modern.minor, 2);
    EXPECT_EQ(modern.profile, GraphicsProfile::Core32);
    EXPECT_FALSE(ChooseContextRequest(GraphicsPlatform::MacOS, false).supported);
}

TEST(GraphicsCapabilities, BuildsEachShaderDialect)
{
    const std::string legacyVertex = BuildVertexShader(ShaderDialect::GLSL120);
    const std::string coreVertex = BuildVertexShader(ShaderDialect::GLSL150Core);
    const std::string coreFragment = BuildFragmentShader(ShaderDialect::GLSL150Core);
    const std::string webFragment = BuildFragmentShader(ShaderDialect::GLSLES100);
    EXPECT_NE(legacyVertex.find("#version 120"), std::string::npos);
    EXPECT_NE(legacyVertex.find("attribute vec3"), std::string::npos);
    EXPECT_NE(coreVertex.find("#version 150 core"), std::string::npos);
    EXPECT_NE(coreVertex.find("in vec3"), std::string::npos);
    EXPECT_EQ(coreVertex.find("attribute"), std::string::npos);
    EXPECT_NE(coreFragment.find("out vec4 fragmentColor"), std::string::npos);
    EXPECT_EQ(coreFragment.find("gl_FragColor"), std::string::npos);
    EXPECT_NE(webFragment.find("precision mediump float"), std::string::npos);
    EXPECT_NE(webFragment.find("texture2D"), std::string::npos);
}
