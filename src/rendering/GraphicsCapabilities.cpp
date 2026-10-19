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
        "uniform bool uTargeting;\n"
        "uniform mat4 uNormalTransform; uniform bool uLit; uniform mat4 uMvp;\nuniform vec4 uDefaultColor;\nuniform bool uHasColor;\n" +
        (core ? "out vec4 vColor;\nout vec2 vUV;\n"
              : "varying vec4 vColor;\nvarying vec2 vUV;\n") +
        "void main() { gl_Position = uMvp * vec4(aPosition, 1.0);"
        // RGB vertex colours have no alpha channel; retain material opacity.
        " vColor = uHasColor ? vec4(aColor, uDefaultColor.a) : uDefaultColor;"
        " if (uTargeting) vColor.rgb *= 1.0 - aPosition.x;"
        " if (uLit) {"
        " vec3 n = mat3(uNormalTransform) * aNormal;"
        " float magnitude = length(n);"
        " float diffuse = magnitude > 0.0 ? max(dot(n / magnitude, normalize(vec3(0.5, 1.0, 0.5))), 0.0) : 0.0;"
        // Fixed-function global ambient (.2) plus LIGHT0 ambient (.2),
        // zero material specular/emission, and diffuse (.8). Clamp per vertex.
        " vColor.rgb = clamp(vColor.rgb * (0.4 + 0.8 * diffuse), 0.0, 1.0); }"
        " vUV = aUV; }\n";
}

std::string BuildFragmentShader(ShaderDialect dialect)
{
    const bool core = dialect == ShaderDialect::GLSL150Core;
    const std::string preamble = dialect == ShaderDialect::GLSLES100 ? "precision mediump float;\n" :
        core ? "#version 150 core\n" : "#version 120\n";
    return preamble +
        (core ? "in vec4 vColor; in vec2 vUV;\nout vec4 fragmentColor;\n"
              : "varying vec4 vColor; varying vec2 vUV;\n") +
        "uniform sampler2D uTexture; uniform bool uHasTexture;\n"
        "uniform bool uTargeting; uniform vec4 uStippleFrame; uniform vec4 uStippleBits;\n"
        "void main() {"
        " if (uTargeting) {"
        " float pixel = max(0.0, dot(gl_FragCoord.xy - uStippleFrame.xy, uStippleFrame.zw));"
        " float bit = mod(floor(pixel / 16.0), 16.0);"
        " float nibble = bit < 4.0 ? uStippleBits.x : bit < 8.0 ? uStippleBits.y : bit < 12.0 ? uStippleBits.z : uStippleBits.w;"
        " if (mod(floor(nibble / exp2(mod(bit, 4.0))), 2.0) < 0.5) discard; }"
        " vec4 texel = uHasTexture ? " +
        (core ? std::string("texture(uTexture, vUV)") : std::string("texture2D(uTexture, vUV)")) +
        " : vec4(1.0); " + (core ? std::string("fragmentColor") : std::string("gl_FragColor")) +
        " = texel * vColor; }\n";
}
