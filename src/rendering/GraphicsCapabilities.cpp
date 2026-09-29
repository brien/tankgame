#include "GraphicsCapabilities.h"

ContextRequest ChooseContextRequest(GraphicsPlatform platform, bool modern)
{
    if (platform == GraphicsPlatform::Web) return {2, 0, GraphicsProfile::ES2, modern};
    if (platform == GraphicsPlatform::MacOS) {
        if (!modern) return {0, 0, GraphicsProfile::Core32, false};
        return {3, 2, GraphicsProfile::Core32, true};
    }
    // Linux and Windows retain a 2.1 compatibility context in both modes. It
    // is the only way the modern slice can coexist with not-yet-migrated code.
    return {2, 1, GraphicsProfile::Compatibility21, true};
}

ShaderDialect ChooseShaderDialect(GraphicsPlatform platform, bool)
{
    if (platform == GraphicsPlatform::Web) return ShaderDialect::GLSLES100;
    if (platform == GraphicsPlatform::MacOS) return ShaderDialect::GLSL150Core;
    return ShaderDialect::GLSL120;
}

std::string BuildVertexShader(ShaderDialect dialect)
{
    const bool core = dialect == ShaderDialect::GLSL150Core;
    const std::string preamble = dialect == ShaderDialect::GLSLES100 ? "precision mediump float;\n" :
        core ? "#version 150 core\n" : "#version 120\n";
    return preamble +
        (core ? "in vec3 aPosition;\nin vec3 aColor;\nin vec2 aUV;\nin vec3 aNormal;\n"
              : "attribute vec3 aPosition;\nattribute vec3 aColor;\nattribute vec2 aUV;\nattribute vec3 aNormal;\n") +
        "uniform mat4 uMvp;\nuniform vec4 uDefaultColor;\nuniform bool uHasColor;\n" +
        (core ? "out vec4 vColor;\nout vec2 vUV;\nout vec3 vNormal;\n"
              : "varying vec4 vColor;\nvarying vec2 vUV;\nvarying vec3 vNormal;\n") +
        "void main() { gl_Position = uMvp * vec4(aPosition, 1.0);"
        " vColor = uHasColor ? vec4(aColor, 1.0) : uDefaultColor;"
        " vUV = aUV; vNormal = aNormal; }\n";
}

std::string BuildFragmentShader(ShaderDialect dialect)
{
    const bool core = dialect == ShaderDialect::GLSL150Core;
    const std::string preamble = dialect == ShaderDialect::GLSLES100 ? "precision mediump float;\n" :
        core ? "#version 150 core\n" : "#version 120\n";
    return preamble +
        (core ? "in vec4 vColor; in vec2 vUV; in vec3 vNormal;\nout vec4 fragmentColor;\n"
              : "varying vec4 vColor; varying vec2 vUV; varying vec3 vNormal;\n") +
        "uniform sampler2D uTexture; uniform bool uHasTexture;\n"
        "void main() { vec4 texel = uHasTexture ? " +
        (core ? std::string("texture(uTexture, vUV)") : std::string("texture2D(uTexture, vUV)")) +
        " : vec4(1.0); " + (core ? std::string("fragmentColor") : std::string("gl_FragColor")) +
        " = texel * vColor + vec4(vNormal.x) * 0.0000001; }\n";
}
