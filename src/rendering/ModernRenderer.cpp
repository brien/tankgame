#include "ModernRenderer.h"

#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

#include "GpuGeometry.h"
#include "GpuTexture.h"
#include "PlatformGL.h"
#include "GLFunctions.h"
#include "GraphicsCapabilities.h"

namespace
{
GLuint CompileShader(GLenum type, const char* source)
{
    const GLuint shader = GLFunctions::CreateShader(type);
    GLFunctions::ShaderSource(shader, 1, &source, nullptr);
    GLFunctions::CompileShader(shader);
    GLint compiled = GL_FALSE;
    GLFunctions::GetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE)
    {
        GLint length = 0;
        GLFunctions::GetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(static_cast<std::size_t>(length > 0 ? length : 1));
        GLFunctions::GetShaderInfoLog(shader, length, nullptr, log.data());
        std::fprintf(stderr, "Modern renderer shader compilation failed: %s\n", log.data());
        GLFunctions::DeleteShader(shader);
        throw std::runtime_error("Modern renderer shader compilation failed");
    }
    return shader;
}
}

class ModernRenderer::ShaderProgram
{
public:
    ShaderProgram()
    {
        GraphicsPlatform platform = GraphicsPlatform::Linux;
#ifdef __EMSCRIPTEN__
        platform = GraphicsPlatform::Web;
#elif defined(__APPLE__)
        platform = GraphicsPlatform::MacOS;
#elif defined(_WIN32)
        platform = GraphicsPlatform::Windows;
#endif
        const ShaderDialect dialect = ChooseShaderDialect(platform, true);
        const std::string vertexSource = BuildVertexShader(dialect);
        const std::string fragmentSource = BuildFragmentShader(dialect);
        const GLuint vertex = CompileShader(GL_VERTEX_SHADER, vertexSource.c_str());
        const GLuint fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource.c_str());
        handle = GLFunctions::CreateProgram();
        GLFunctions::AttachShader(handle, vertex);
        GLFunctions::AttachShader(handle, fragment);
        GLFunctions::BindAttribLocation(handle, 0, "aPosition");
        GLFunctions::BindAttribLocation(handle, 1, "aColor");
        GLFunctions::BindAttribLocation(handle, 2, "aUV");
        GLFunctions::BindAttribLocation(handle, 3, "aNormal");
        GLFunctions::LinkProgram(handle);
        GLFunctions::DeleteShader(vertex);
        GLFunctions::DeleteShader(fragment);
        GLint linked = GL_FALSE;
        GLFunctions::GetProgramiv(handle, GL_LINK_STATUS, &linked);
        if (linked == GL_FALSE)
        {
            GLint length = 0;
            GLFunctions::GetProgramiv(handle, GL_INFO_LOG_LENGTH, &length);
            std::vector<char> log(static_cast<std::size_t>(length > 0 ? length : 1));
            GLFunctions::GetProgramInfoLog(handle, length, nullptr, log.data());
            std::fprintf(stderr, "Modern renderer program link failed: %s\n", log.data());
            GLFunctions::DeleteProgram(handle);
            handle = 0;
            throw std::runtime_error("Modern renderer program link failed");
        }
        mvp = GLFunctions::GetUniformLocation(handle, "uMvp");
        defaultColor = GLFunctions::GetUniformLocation(handle, "uDefaultColor");
        hasColor = GLFunctions::GetUniformLocation(handle, "uHasColor");
        hasTexture = GLFunctions::GetUniformLocation(handle, "uHasTexture");
        texture = GLFunctions::GetUniformLocation(handle, "uTexture");
    }

    ~ShaderProgram() { if (handle != 0) GLFunctions::DeleteProgram(handle); }

    GLuint handle = 0;
    GLint mvp = -1;
    GLint defaultColor = -1;
    GLint hasColor = -1;
    GLint hasTexture = -1;
    GLint texture = -1;
};

ModernRenderer::ModernRenderer() : program(new ShaderProgram) {}
ModernRenderer::~ModernRenderer() = default;

void ModernRenderer::Draw(const GpuGeometry& geometry, const float* mvp,
                          const BasicMaterial& material) const
{
    if (mvp == nullptr) throw std::invalid_argument("ModernRenderer MVP matrix cannot be null");
    const GeometryAttributeLayout& layout = geometry.Layout();
    GLFunctions::UseProgram(program->handle);
    GLFunctions::BindBuffer(GL_ARRAY_BUFFER, geometry.BufferHandle());
    const GLsizei stride = static_cast<GLsizei>(layout.strideFloats * sizeof(float));
    GLFunctions::EnableVertexAttribArray(0);
    GLFunctions::VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    const auto optional = [&](GLuint index, bool present, GLint size, std::size_t offset) {
        if (present) {
            GLFunctions::EnableVertexAttribArray(index);
            GLFunctions::VertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride,
                                  reinterpret_cast<const void*>(offset * sizeof(float)));
        } else GLFunctions::DisableVertexAttribArray(index);
    };
    optional(1, layout.hasColors, 3, layout.colorOffset);
    optional(2, layout.hasTextureCoordinates, 2, layout.textureCoordinateOffset);
    optional(3, layout.hasNormals, 3, layout.normalOffset);

    const GLfloat color[] = {material.red, material.green, material.blue, material.alpha};
    GLFunctions::UniformMatrix4fv(program->mvp, 1, GL_FALSE, mvp);
    GLFunctions::Uniform4fv(program->defaultColor, 1, color);
    GLFunctions::Uniform1i(program->hasColor, layout.hasColors ? 1 : 0);
    const bool textured = material.texture && layout.hasTextureCoordinates;
    GLFunctions::Uniform1i(program->hasTexture, textured ? 1 : 0);
    GLFunctions::Uniform1i(program->texture, 0);
    if (textured) material.texture->Bind(0);
    const GLenum mode = geometry.DrawMode() == GeometryDrawMode::LINES ? GL_LINES :
        geometry.DrawMode() == GeometryDrawMode::LINE_LOOP ? GL_LINE_LOOP : GL_TRIANGLES;
    glDrawArrays(mode, 0, static_cast<GLsizei>(geometry.VertexCount()));
    GLFunctions::BindBuffer(GL_ARRAY_BUFFER, 0);
}

const void* ModernRenderer::ProgramIdentity() const { return program.get(); }
