#include "DisplayList.h"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>

#ifdef __EMSCRIPTEN__
#include <GLES2/gl2.h>
#include "GeometryBuffer.h"
#endif

#ifndef __EMSCRIPTEN__
#ifdef _WIN32
#pragma warning(disable : 4996)
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#elif __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif

class DisplayList::Implementation
{
public:
#ifdef __EMSCRIPTEN__
    explicit Implementation(int count) : count(count) {}

    ~Implementation()
    {
        if (vertexBuffer != 0)
            glDeleteBuffers(1, &vertexBuffer);
        if (program != 0)
            glDeleteProgram(program);
    }

    int count;
    Geometry geometry;
    GLuint vertexBuffer = 0;
    GLuint program = 0;
    GLenum drawMode = GL_TRIANGLES;
    GLsizei vertexCount = 0;
    GeometryAttributeLayout layout;
    GLfloat mvp[16] = {1, 0, 0, 0, 0, 1, 0, 0,
                       0, 0, 1, 0, 0, 0, 0, 1};
    GLfloat defaultColor[4] = {1, 1, 1, 1};
#else
    explicit Implementation(int count)
        : count(count), first(count > 0 ? glGenLists(count) : 0), current(first)
    {
    }

    ~Implementation()
    {
        if (first != 0)
            glDeleteLists(first, count);
    }

    int count;
    GLuint first;
    GLuint current;
    Geometry geometry;
#endif
};

#ifdef __EMSCRIPTEN__
namespace
{
[[noreturn]] void UnsupportedDisplayListOperation(const char* operation)
{
    std::fprintf(stderr,
        "DisplayList::%s cannot run on WebGL: geometry must be converted to "
        "backend-neutral vertex data first.\n",
        operation);
    std::abort();
}

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
        std::fprintf(stderr, "Geometry WebGL shader compilation failed: %s\n", log.data());
        glDeleteShader(shader);
        throw std::runtime_error("Geometry WebGL shader compilation failed");
    }
    return shader;
}

GLuint CreateProgram()
{
    static const char* vertexSource =
        "attribute vec3 aPosition;\n"
        "attribute vec3 aColor;\n"
        "attribute vec2 aUV;\n"
        "attribute vec3 aNormal;\n"
        "uniform mat4 uMvp;\n"
        "uniform vec4 uDefaultColor;\n"
        "uniform bool uHasColor;\n"
        "varying vec4 vColor;\n"
        "varying vec2 vUV;\n"
        "varying vec3 vNormal;\n"
        "void main() { gl_Position = uMvp * vec4(aPosition, 1.0);"
        " vColor = uHasColor ? vec4(aColor, 1.0) : uDefaultColor;"
        " vUV = aUV; vNormal = aNormal; }\n";
    static const char* fragmentSource =
        "precision mediump float;\n"
        "varying vec4 vColor; varying vec2 vUV; varying vec3 vNormal;\n"
        "void main() { gl_FragColor = vColor + vec4(vUV, vNormal.x, 0.0) * 0.0000001; }\n";
    const GLuint vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
    const GLuint fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glBindAttribLocation(program, 0, "aPosition");
    glBindAttribLocation(program, 1, "aColor");
    glBindAttribLocation(program, 2, "aUV");
    glBindAttribLocation(program, 3, "aNormal");
    glLinkProgram(program);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE)
    {
        GLint length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(static_cast<std::size_t>(length > 0 ? length : 1));
        glGetProgramInfoLog(program, length, nullptr, log.data());
        std::fprintf(stderr, "Geometry WebGL program link failed: %s\n", log.data());
        glDeleteProgram(program);
        throw std::runtime_error("Geometry WebGL program link failed");
    }
    return program;
}
}
#endif

DisplayList::DisplayList(int num)
    : implementation(std::make_shared<Implementation>(num))
{
}

void DisplayList::BeginNewList()
{
    ResetList();
    NewList();
}

void DisplayList::NextNewList()
{
#ifdef __EMSCRIPTEN__
    UnsupportedDisplayListOperation("NextNewList");
#else
    glEndList();
    ++implementation->current;
    if (implementation->current >= implementation->first + implementation->count ||
        implementation->current < implementation->first)
        return;
    glNewList(implementation->current, GL_COMPILE);
#endif
}

void DisplayList::EndNewList()
{
#ifdef __EMSCRIPTEN__
    UnsupportedDisplayListOperation("EndNewList");
#else
    glEndList();
#endif
}

void DisplayList::ResetList()
{
#ifdef __EMSCRIPTEN__
    // Resetting the cursor does not claim that the resource can be rendered.
#else
    implementation->current = implementation->first;
#endif
}

void DisplayList::NewList()
{
#ifdef __EMSCRIPTEN__
    UnsupportedDisplayListOperation("NewList");
#else
    glNewList(implementation->current, GL_COMPILE);
#endif
}

void DisplayList::EndList()
{
#ifdef __EMSCRIPTEN__
    UnsupportedDisplayListOperation("EndList");
#else
    glEndList();
    ++implementation->current;
#endif
}

void DisplayList::SetGeometry(const Geometry& geometry)
{
    implementation->geometry = geometry;
#ifdef __EMSCRIPTEN__
    const PreparedGeometry prepared = PrepareGeometryForGpu(geometry);
    implementation->layout = prepared.layout;
    implementation->vertexCount = static_cast<GLsizei>(prepared.VertexCount());
    implementation->drawMode = prepared.drawMode == GeometryDrawMode::LINES ? GL_LINES :
        prepared.drawMode == GeometryDrawMode::LINE_LOOP ? GL_LINE_LOOP : GL_TRIANGLES;
    if (implementation->program == 0)
        implementation->program = CreateProgram();
    if (implementation->vertexBuffer == 0)
        glGenBuffers(1, &implementation->vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, implementation->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(prepared.vertices.size() * sizeof(float)),
                 prepared.vertices.empty() ? nullptr : prepared.vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#else
    BeginNewList();
    GLenum mode = GL_QUADS;
    if (geometry.topology == PrimitiveTopology::LINE_LOOP)
        mode = GL_LINE_LOOP;
    else if (geometry.topology == PrimitiveTopology::LINES)
        mode = GL_LINES;
    else if (geometry.topology == PrimitiveTopology::TRIANGLES)
        mode = GL_TRIANGLES;

    glBegin(mode);
    for (const GeometryVertex& vertex : geometry.vertices)
    {
        if (geometry.hasColors)
            glColor3f(vertex.red, vertex.green, vertex.blue);
        if (geometry.hasNormals)
            glNormal3f(vertex.normalX, vertex.normalY, vertex.normalZ);
        if (geometry.hasTextureCoordinates)
            glTexCoord2f(vertex.u, vertex.v);
        glVertex3f(vertex.x, vertex.y, vertex.z);
    }
    glEnd();
    EndNewList();
#endif
}

void DisplayList::Call(int i)
{
#ifdef __EMSCRIPTEN__
    if (i != 0 || i >= implementation->count)
        throw std::out_of_range("DisplayList geometry resource index is out of range");
    if (implementation->vertexBuffer == 0 || implementation->program == 0)
        throw std::runtime_error("DisplayList::Call has no uploaded Geometry resource");

    glUseProgram(implementation->program);
    glBindBuffer(GL_ARRAY_BUFFER, implementation->vertexBuffer);
    const GLsizei stride = static_cast<GLsizei>(implementation->layout.strideFloats * sizeof(float));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);

    const auto configureOptional = [&](GLuint index, bool present, GLint size, std::size_t offset) {
        if (present)
        {
            glEnableVertexAttribArray(index);
            glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride,
                                  reinterpret_cast<const void*>(offset * sizeof(float)));
        }
        else
            glDisableVertexAttribArray(index);
    };
    configureOptional(1, implementation->layout.hasColors, 3, implementation->layout.colorOffset);
    configureOptional(2, implementation->layout.hasTextureCoordinates, 2,
                      implementation->layout.textureCoordinateOffset);
    configureOptional(3, implementation->layout.hasNormals, 3, implementation->layout.normalOffset);

    glUniformMatrix4fv(glGetUniformLocation(implementation->program, "uMvp"), 1, GL_FALSE,
                       implementation->mvp);
    glUniform4fv(glGetUniformLocation(implementation->program, "uDefaultColor"), 1,
                  implementation->defaultColor);
    glUniform1i(glGetUniformLocation(implementation->program, "uHasColor"),
                implementation->layout.hasColors ? 1 : 0);
    glDrawArrays(implementation->drawMode, 0, implementation->vertexCount);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#else
    glCallList(implementation->first + i);
#endif
}

void DisplayList::SetMvpMatrix(const float* columnMajorMatrix)
{
#ifdef __EMSCRIPTEN__
    if (columnMajorMatrix == nullptr)
        throw std::invalid_argument("DisplayList MVP matrix cannot be null");
    for (int i = 0; i < 16; ++i)
        implementation->mvp[i] = columnMajorMatrix[i];
#else
    (void)columnMajorMatrix;
#endif
}

void DisplayList::SetDefaultColor(float red, float green, float blue, float alpha)
{
#ifdef __EMSCRIPTEN__
    implementation->defaultColor[0] = red;
    implementation->defaultColor[1] = green;
    implementation->defaultColor[2] = blue;
    implementation->defaultColor[3] = alpha;
#else
    (void)red;
    (void)green;
    (void)blue;
    (void)alpha;
#endif
}

void DisplayList::Close()
{
    implementation.reset();
}
