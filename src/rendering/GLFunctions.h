#pragma once

#include <string>

#include "PlatformGL.h"

// The complete entry-point boundary used by the shared programmable renderer.
// Initialize must be called with the SDL GL context current. OpenGL 1.1 state,
// texture upload and draw calls remain direct because every desktop ABI exports
// them; GLES/WebGL keeps using its normal linked symbols.
namespace GLFunctions
{
bool Initialize(std::string& error);
void Shutdown();
bool IsInitialized();

GLuint CreateShader(GLenum type);
void ShaderSource(GLuint shader, GLsizei count, const GLchar* const* source, const GLint* length);
void CompileShader(GLuint shader);
void GetShaderiv(GLuint shader, GLenum pname, GLint* value);
void GetShaderInfoLog(GLuint shader, GLsizei size, GLsizei* length, GLchar* log);
void DeleteShader(GLuint shader);
GLuint CreateProgram();
void AttachShader(GLuint program, GLuint shader);
void BindAttribLocation(GLuint program, GLuint index, const GLchar* name);
void LinkProgram(GLuint program);
void GetProgramiv(GLuint program, GLenum pname, GLint* value);
void GetProgramInfoLog(GLuint program, GLsizei size, GLsizei* length, GLchar* log);
void DeleteProgram(GLuint program);
GLint GetUniformLocation(GLuint program, const GLchar* name);
void UseProgram(GLuint program);
void GenBuffers(GLsizei count, GLuint* buffers);
void BindBuffer(GLenum target, GLuint buffer);
void BufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
void DeleteBuffers(GLsizei count, const GLuint* buffers);
void EnableVertexAttribArray(GLuint index);
void DisableVertexAttribArray(GLuint index);
void VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized,
                         GLsizei stride, const void* pointer);
void UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void Uniform4fv(GLint location, GLsizei count, const GLfloat* value);
void Uniform1i(GLint location, GLint value);
void ActiveTexture(GLenum texture);
void GenerateMipmap(GLenum target);
}
