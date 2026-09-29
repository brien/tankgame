#include "GpuGeometry.h"

#include "PlatformGL.h"

class GpuGeometry::Implementation
{
public:
    GLuint buffer = 0;
    GeometryAttributeLayout layout;
    GeometryDrawMode drawMode = GeometryDrawMode::TRIANGLES;
    std::size_t vertexCount = 0;
};

GpuGeometry::GpuGeometry(const Geometry& geometry) : implementation(new Implementation)
{
    const PreparedGeometry prepared = PrepareGeometryForGpu(geometry);
    implementation->layout = prepared.layout;
    implementation->drawMode = prepared.drawMode;
    implementation->vertexCount = prepared.VertexCount();
    glGenBuffers(1, &implementation->buffer);
    glBindBuffer(GL_ARRAY_BUFFER, implementation->buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(prepared.vertices.size() * sizeof(float)),
                 prepared.vertices.empty() ? nullptr : prepared.vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GpuGeometry::~GpuGeometry()
{
    if (implementation->buffer != 0)
        glDeleteBuffers(1, &implementation->buffer);
}

const GeometryAttributeLayout& GpuGeometry::Layout() const { return implementation->layout; }
std::size_t GpuGeometry::VertexCount() const { return implementation->vertexCount; }
GeometryDrawMode GpuGeometry::DrawMode() const { return implementation->drawMode; }
unsigned int GpuGeometry::BufferHandle() const { return implementation->buffer; }
