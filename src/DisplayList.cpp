#include "DisplayList.h"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include "rendering/GpuGeometry.h"
#include "rendering/PlatformGL.h"
#include "rendering/RendererMode.h"

class DisplayList::Implementation
{
public:
    explicit Implementation(int count) : count(count) {}

    ~Implementation()
    {
#ifndef __EMSCRIPTEN__
        if (first != 0)
            glDeleteLists(first, count);
#endif
    }

    int count;
    Geometry geometry;
    std::shared_ptr<GpuGeometry> gpuGeometry;
#ifndef __EMSCRIPTEN__
    GLuint first = 0;
    GLuint current = 0;
#endif
};

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

}

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
    if (RendererMode::IsModern())
        UnsupportedDisplayListOperation("NextNewList");
#ifndef __EMSCRIPTEN__
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
    if (RendererMode::IsModern())
        UnsupportedDisplayListOperation("EndNewList");
#ifndef __EMSCRIPTEN__
    glEndList();
#endif
}

void DisplayList::ResetList()
{
    if (RendererMode::IsModern())
        return;
#ifndef __EMSCRIPTEN__
    implementation->current = implementation->first;
#endif
}

void DisplayList::NewList()
{
    if (RendererMode::IsModern())
        UnsupportedDisplayListOperation("NewList");
#ifndef __EMSCRIPTEN__
    glNewList(implementation->current, GL_COMPILE);
#endif
}

void DisplayList::EndList()
{
    if (RendererMode::IsModern())
        UnsupportedDisplayListOperation("EndList");
#ifndef __EMSCRIPTEN__
    glEndList();
    ++implementation->current;
#endif
}

void DisplayList::SetGeometry(const Geometry& geometry)
{
    implementation->geometry = geometry;
    if (RendererMode::IsModern())
    {
        implementation->gpuGeometry = std::make_shared<GpuGeometry>(geometry);
        return;
    }
#ifndef __EMSCRIPTEN__
    // Compatibility names are allocated lazily.  This keeps construction of
    // the resource catalogue CPU-only; initialization after context creation
    // is the single point where compatibility GPU objects are created.
    if (implementation->first == 0 && implementation->count > 0)
        implementation->first = implementation->current =
            glGenLists(implementation->count);
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
    if (RendererMode::IsModern())
        throw std::logic_error("DisplayList::Call is compatibility-only; use RenderContext::Draw");
#ifndef __EMSCRIPTEN__
    glCallList(implementation->first + i);
#endif
}

const GpuGeometry& DisplayList::ModernGeometry() const
{
    if (!implementation || !implementation->gpuGeometry)
        throw std::runtime_error("DisplayList has no uploaded modern geometry");
    return *implementation->gpuGeometry;
}

void DisplayList::Close()
{
    implementation.reset();
}
