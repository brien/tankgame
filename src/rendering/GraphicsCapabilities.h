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
};

ContextRequest ChooseContextRequest(GraphicsPlatform platform);
ShaderDialect ChooseShaderDialect(GraphicsPlatform platform);
std::string BuildVertexShader(ShaderDialect dialect);
std::string BuildFragmentShader(ShaderDialect dialect);
