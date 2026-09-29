#include "GLFunctions.h"

#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
namespace GLFunctions {
bool Initialize(std::string&) { return true; }
void Shutdown() {}
bool IsInitialized() { return true; }
#define TG_DIRECT(ret, name, args, call) ret name args { return gl##name call; }
TG_DIRECT(GLuint, CreateShader, (GLenum a), (a))
TG_DIRECT(void, ShaderSource, (GLuint a, GLsizei b, const GLchar* const* c, const GLint* d), (a,b,c,d))
TG_DIRECT(void, CompileShader, (GLuint a), (a)) TG_DIRECT(void, GetShaderiv, (GLuint a, GLenum b, GLint* c), (a,b,c))
TG_DIRECT(void, GetShaderInfoLog, (GLuint a, GLsizei b, GLsizei* c, GLchar* d), (a,b,c,d)) TG_DIRECT(void, DeleteShader, (GLuint a), (a))
TG_DIRECT(GLuint, CreateProgram, (), ()) TG_DIRECT(void, AttachShader, (GLuint a, GLuint b), (a,b))
TG_DIRECT(void, BindAttribLocation, (GLuint a, GLuint b, const GLchar* c), (a,b,c)) TG_DIRECT(void, LinkProgram, (GLuint a), (a))
TG_DIRECT(void, GetProgramiv, (GLuint a, GLenum b, GLint* c), (a,b,c)) TG_DIRECT(void, GetProgramInfoLog, (GLuint a, GLsizei b, GLsizei* c, GLchar* d), (a,b,c,d))
TG_DIRECT(void, DeleteProgram, (GLuint a), (a)) TG_DIRECT(GLint, GetUniformLocation, (GLuint a, const GLchar* b), (a,b)) TG_DIRECT(void, UseProgram, (GLuint a), (a))
TG_DIRECT(void, GenBuffers, (GLsizei a, GLuint* b), (a,b)) TG_DIRECT(void, BindBuffer, (GLenum a, GLuint b), (a,b))
TG_DIRECT(void, BufferData, (GLenum a, GLsizeiptr b, const void* c, GLenum d), (a,b,c,d)) TG_DIRECT(void, DeleteBuffers, (GLsizei a, const GLuint* b), (a,b))
TG_DIRECT(void, EnableVertexAttribArray, (GLuint a), (a)) TG_DIRECT(void, DisableVertexAttribArray, (GLuint a), (a))
TG_DIRECT(void, VertexAttribPointer, (GLuint a, GLint b, GLenum c, GLboolean d, GLsizei e, const void* f), (a,b,c,d,e,f))
TG_DIRECT(void, UniformMatrix4fv, (GLint a, GLsizei b, GLboolean c, const GLfloat* d), (a,b,c,d)) TG_DIRECT(void, Uniform4fv, (GLint a, GLsizei b, const GLfloat* c), (a,b,c))
TG_DIRECT(void, Uniform1i, (GLint a, GLint b), (a,b)) TG_DIRECT(void, ActiveTexture, (GLenum a), (a)) TG_DIRECT(void, GenerateMipmap, (GLenum a), (a))
#undef TG_DIRECT
}
#else
namespace {
bool initialized = false;
#define TG_PROC(ret, name, args) using name##Proc = ret (APIENTRYP) args; name##Proc p##name = nullptr
TG_PROC(GLuint, CreateShader, (GLenum)); TG_PROC(void, ShaderSource, (GLuint, GLsizei, const GLchar* const*, const GLint*));
TG_PROC(void, CompileShader, (GLuint)); TG_PROC(void, GetShaderiv, (GLuint, GLenum, GLint*)); TG_PROC(void, GetShaderInfoLog, (GLuint, GLsizei, GLsizei*, GLchar*)); TG_PROC(void, DeleteShader, (GLuint));
TG_PROC(GLuint, CreateProgram, ()); TG_PROC(void, AttachShader, (GLuint, GLuint)); TG_PROC(void, BindAttribLocation, (GLuint, GLuint, const GLchar*)); TG_PROC(void, LinkProgram, (GLuint));
TG_PROC(void, GetProgramiv, (GLuint, GLenum, GLint*)); TG_PROC(void, GetProgramInfoLog, (GLuint, GLsizei, GLsizei*, GLchar*)); TG_PROC(void, DeleteProgram, (GLuint)); TG_PROC(GLint, GetUniformLocation, (GLuint, const GLchar*)); TG_PROC(void, UseProgram, (GLuint));
TG_PROC(void, GenBuffers, (GLsizei, GLuint*)); TG_PROC(void, BindBuffer, (GLenum, GLuint)); TG_PROC(void, BufferData, (GLenum, GLsizeiptr, const void*, GLenum)); TG_PROC(void, DeleteBuffers, (GLsizei, const GLuint*));
TG_PROC(void, EnableVertexAttribArray, (GLuint)); TG_PROC(void, DisableVertexAttribArray, (GLuint)); TG_PROC(void, VertexAttribPointer, (GLuint, GLint, GLenum, GLboolean, GLsizei, const void*));
TG_PROC(void, UniformMatrix4fv, (GLint, GLsizei, GLboolean, const GLfloat*)); TG_PROC(void, Uniform4fv, (GLint, GLsizei, const GLfloat*)); TG_PROC(void, Uniform1i, (GLint, GLint));
TG_PROC(void, ActiveTexture, (GLenum)); TG_PROC(void, GenerateMipmap, (GLenum));
#undef TG_PROC

template <typename T> bool Load(T& target, const char* name, std::string& missing)
{
    target = reinterpret_cast<T>(SDL_GL_GetProcAddress(name));
    if (target) return true;
    if (!missing.empty()) missing += ", ";
    missing += name;
    return false;
}
}
namespace GLFunctions {
bool Initialize(std::string& error)
{
    if (initialized) return true;
    std::string missing;
#define TG_LOAD(name) Load(p##name, "gl" #name, missing)
    TG_LOAD(CreateShader); TG_LOAD(ShaderSource); TG_LOAD(CompileShader); TG_LOAD(GetShaderiv); TG_LOAD(GetShaderInfoLog); TG_LOAD(DeleteShader);
    TG_LOAD(CreateProgram); TG_LOAD(AttachShader); TG_LOAD(BindAttribLocation); TG_LOAD(LinkProgram); TG_LOAD(GetProgramiv); TG_LOAD(GetProgramInfoLog); TG_LOAD(DeleteProgram); TG_LOAD(GetUniformLocation); TG_LOAD(UseProgram);
    TG_LOAD(GenBuffers); TG_LOAD(BindBuffer); TG_LOAD(BufferData); TG_LOAD(DeleteBuffers); TG_LOAD(EnableVertexAttribArray); TG_LOAD(DisableVertexAttribArray); TG_LOAD(VertexAttribPointer);
    TG_LOAD(UniformMatrix4fv); TG_LOAD(Uniform4fv); TG_LOAD(Uniform1i); TG_LOAD(ActiveTexture);
    pGenerateMipmap = reinterpret_cast<GenerateMipmapProc>(SDL_GL_GetProcAddress("glGenerateMipmap"));
    if (!pGenerateMipmap) {
        // GL 2.1 implementations commonly expose this required operation via
        // EXT_framebuffer_object rather than the later core spelling.
        Load(pGenerateMipmap, "glGenerateMipmapEXT", missing);
    }
#undef TG_LOAD
    if (!missing.empty()) { error = "Missing required modern OpenGL functions: " + missing; return false; }
    initialized = true; return true;
}
void Shutdown() { initialized = false; }
bool IsInitialized() { return initialized; }
#define TG_FORWARD(ret, name, args, call) ret name args { return p##name call; }
TG_FORWARD(GLuint, CreateShader, (GLenum a), (a)) TG_FORWARD(void, ShaderSource, (GLuint a, GLsizei b, const GLchar* const* c, const GLint* d), (a,b,c,d))
TG_FORWARD(void, CompileShader, (GLuint a), (a)) TG_FORWARD(void, GetShaderiv, (GLuint a, GLenum b, GLint* c), (a,b,c)) TG_FORWARD(void, GetShaderInfoLog, (GLuint a, GLsizei b, GLsizei* c, GLchar* d), (a,b,c,d)) TG_FORWARD(void, DeleteShader, (GLuint a), (a))
TG_FORWARD(GLuint, CreateProgram, (), ()) TG_FORWARD(void, AttachShader, (GLuint a, GLuint b), (a,b)) TG_FORWARD(void, BindAttribLocation, (GLuint a, GLuint b, const GLchar* c), (a,b,c)) TG_FORWARD(void, LinkProgram, (GLuint a), (a))
TG_FORWARD(void, GetProgramiv, (GLuint a, GLenum b, GLint* c), (a,b,c)) TG_FORWARD(void, GetProgramInfoLog, (GLuint a, GLsizei b, GLsizei* c, GLchar* d), (a,b,c,d)) TG_FORWARD(void, DeleteProgram, (GLuint a), (a)) TG_FORWARD(GLint, GetUniformLocation, (GLuint a, const GLchar* b), (a,b)) TG_FORWARD(void, UseProgram, (GLuint a), (a))
TG_FORWARD(void, GenBuffers, (GLsizei a, GLuint* b), (a,b)) TG_FORWARD(void, BindBuffer, (GLenum a, GLuint b), (a,b)) TG_FORWARD(void, BufferData, (GLenum a, GLsizeiptr b, const void* c, GLenum d), (a,b,c,d)) TG_FORWARD(void, DeleteBuffers, (GLsizei a, const GLuint* b), (a,b))
TG_FORWARD(void, EnableVertexAttribArray, (GLuint a), (a)) TG_FORWARD(void, DisableVertexAttribArray, (GLuint a), (a)) TG_FORWARD(void, VertexAttribPointer, (GLuint a, GLint b, GLenum c, GLboolean d, GLsizei e, const void* f), (a,b,c,d,e,f))
TG_FORWARD(void, UniformMatrix4fv, (GLint a, GLsizei b, GLboolean c, const GLfloat* d), (a,b,c,d)) TG_FORWARD(void, Uniform4fv, (GLint a, GLsizei b, const GLfloat* c), (a,b,c)) TG_FORWARD(void, Uniform1i, (GLint a, GLint b), (a,b)) TG_FORWARD(void, ActiveTexture, (GLenum a), (a)) TG_FORWARD(void, GenerateMipmap, (GLenum a), (a))
#undef TG_FORWARD
}
#endif
