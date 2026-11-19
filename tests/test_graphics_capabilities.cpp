#include <gtest/gtest.h>

#include "rendering/GraphicsCapabilities.h"

TEST(GraphicsCapabilities, PreservesDesktopHardwareBaseline)
{
    for (const auto platform : {GraphicsPlatform::Linux, GraphicsPlatform::Windows}) {
        const auto context = ChooseContextRequest(platform);
        EXPECT_EQ(context.major, 2);
        EXPECT_EQ(context.minor, 1);
        EXPECT_EQ(context.profile, GraphicsProfile::Compatibility21);
        EXPECT_EQ(ChooseShaderDialect(platform), ShaderDialect::GLSL120);
    }
}

TEST(GraphicsCapabilities, UsesMacCoreProfile)
{
    const auto context = ChooseContextRequest(GraphicsPlatform::MacOS);
    EXPECT_EQ(context.major, 3);
    EXPECT_EQ(context.minor, 2);
    EXPECT_EQ(context.profile, GraphicsProfile::Core32);
    EXPECT_EQ(ChooseShaderDialect(GraphicsPlatform::MacOS), ShaderDialect::GLSL150Core);
}

TEST(GraphicsCapabilities, UsesWebGLES2Profile)
{
    const auto context = ChooseContextRequest(GraphicsPlatform::Web);
    EXPECT_EQ(context.major, 2);
    EXPECT_EQ(context.minor, 0);
    EXPECT_EQ(context.profile, GraphicsProfile::ES2);
    EXPECT_EQ(ChooseShaderDialect(GraphicsPlatform::Web), ShaderDialect::GLSLES100);
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

TEST(GraphicsCapabilities, WebTransformsUseHighPrecisionWithPortableVaryings)
{
    const std::string vertex = BuildVertexShader(ShaderDialect::GLSLES100);
    const std::string fragment = BuildFragmentShader(ShaderDialect::GLSLES100);
    EXPECT_NE(vertex.find("precision highp float;"), std::string::npos);
    // Keep inter-stage precision compatible with GLES2 fragment implementations
    // that only support mediump floats. The transform itself stays highp.
    EXPECT_NE(vertex.find("varying mediump vec4 vColor;"), std::string::npos);
    EXPECT_NE(vertex.find("varying mediump vec2 vUV;"), std::string::npos);
    EXPECT_NE(fragment.find("precision mediump float;"), std::string::npos);
}
