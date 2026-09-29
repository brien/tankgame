#include "ModernRenderer.h"

#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

#include "GpuGeometry.h"
#include "GpuTexture.h"
#include "PlatformGL.h"

namespace
{
GLuint CompileShader(GLenum type, const char* source)
{
    const GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE)
    {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(static_cast<std::size_t>(length > 0 ? length : 1));
        glGetShaderInfoLog(shader, length, nullptr, log.data());
        std::fprintf(stderr, "Modern renderer shader compilation failed: %s\n", log.data());
        glDeleteShader(shader);
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
        static const char* vertexBody =
            "attribute vec3 aPosition;\nattribute vec3 aColor;\nattribute vec2 aUV;\n"
            "attribute vec3 aNormal;\nuniform mat4 uMvp;\nuniform vec4 uDefaultColor;\n"
            "uniform bool uHasColor;\nvarying vec4 vColor;\nvarying vec2 vUV;\n"
            "varying vec3 vNormal;\nvoid main() { gl_Position = uMvp * vec4(aPosition, 1.0);"
            " vColor = uHasColor ? vec4(aColor, 1.0) : uDefaultColor;"
            " vUV = aUV; vNormal = aNormal; }\n";
        static const char* fragmentBody =
            "varying vec4 vColor; varying vec2 vUV; varying vec3 vNormal;\n"
            "uniform sampler2D uTexture; uniform bool uHasTexture;\n"
            "void main() { vec4 texel = uHasTexture ? texture2D(uTexture, vUV) : vec4(1.0);"
            " gl_FragColor = texel * vColor + vec4(vNormal.x) * 0.0000001; }\n";
#ifdef __EMSCRIPTEN__
        const std::string preamble = "precision mediump float;\n";
#else
        const std::string preamble = "#version 120\n";
#endif
        const std::string vertexSource = preamble + vertexBody;
        const std::string fragmentSource = preamble + fragmentBody;
        const GLuint vertex = CompileShader(GL_VERTEX_SHADER, vertexSource.c_str());
        const GLuint fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource.c_str());
        handle = glCreateProgram();
        glAttachShader(handle, vertex);
        glAttachShader(handle, fragment);
        glBindAttribLocation(handle, 0, "aPosition");
        glBindAttribLocation(handle, 1, "aColor");
        glBindAttribLocation(handle, 2, "aUV");
        glBindAttribLocation(handle, 3, "aNormal");
        glLinkProgram(handle);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        GLint linked = GL_FALSE;
        glGetProgramiv(handle, GL_LINK_STATUS, &linked);
        if (linked == GL_FALSE)
        {
            GLint length = 0;
            glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &length);
            std::vector<char> log(static_cast<std::size_t>(length > 0 ? length : 1));
            glGetProgramInfoLog(handle, length, nullptr, log.data());
            std::fprintf(stderr, "Modern renderer program link failed: %s\n", log.data());
            glDeleteProgram(handle);
            handle = 0;
            throw std::runtime_error("Modern renderer program link failed");
        }
        mvp = glGetUniformLocation(handle, "uMvp");
        defaultColor = glGetUniformLocation(handle, "uDefaultColor");
        hasColor = glGetUniformLocation(handle, "uHasColor");
        hasTexture = glGetUniformLocation(handle, "uHasTexture");
        texture = glGetUniformLocation(handle, "uTexture");
    }

    ~ShaderProgram() { if (handle != 0) glDeleteProgram(handle); }

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
    glUseProgram(program->handle);
    glBindBuffer(GL_ARRAY_BUFFER, geometry.BufferHandle());
    const GLsizei stride = static_cast<GLsizei>(layout.strideFloats * sizeof(float));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    const auto optional = [&](GLuint index, bool present, GLint size, std::size_t offset) {
        if (present) {
            glEnableVertexAttribArray(index);
            glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride,
                                  reinterpret_cast<const void*>(offset * sizeof(float)));
        } else glDisableVertexAttribArray(index);
    };
    optional(1, layout.hasColors, 3, layout.colorOffset);
    optional(2, layout.hasTextureCoordinates, 2, layout.textureCoordinateOffset);
    optional(3, layout.hasNormals, 3, layout.normalOffset);

    const GLfloat color[] = {material.red, material.green, material.blue, material.alpha};
    glUniformMatrix4fv(program->mvp, 1, GL_FALSE, mvp);
    glUniform4fv(program->defaultColor, 1, color);
    glUniform1i(program->hasColor, layout.hasColors ? 1 : 0);
    const bool textured = material.texture && layout.hasTextureCoordinates;
    glUniform1i(program->hasTexture, textured ? 1 : 0);
    glUniform1i(program->texture, 0);
    if (textured) material.texture->Bind(0);
    const GLenum mode = geometry.DrawMode() == GeometryDrawMode::LINES ? GL_LINES :
        geometry.DrawMode() == GeometryDrawMode::LINE_LOOP ? GL_LINE_LOOP : GL_TRIANGLES;
    glDrawArrays(mode, 0, static_cast<GLsizei>(geometry.VertexCount()));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

const void* ModernRenderer::ProgramIdentity() const { return program.get(); }
