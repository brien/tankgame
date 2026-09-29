#pragma once

#include <string>

enum class GraphicsPlatform { Linux, Windows, MacOS, Web };
enum class GraphicsProfile { Compatibility21, Core32, ES2 };
enum class ShaderDialect { GLSL120, GLSL150Core, GLSLES100 };

struct ContextRequest
{
    int major;
    int minor;
    GraphicsProfile profile;
    bool supported;
};

ContextRequest ChooseContextRequest(GraphicsPlatform platform, bool modernRenderer);
ShaderDialect ChooseShaderDialect(GraphicsPlatform platform, bool modernRenderer);
std::string BuildVertexShader(ShaderDialect dialect);
std::string BuildFragmentShader(ShaderDialect dialect);
