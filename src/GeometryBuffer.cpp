#include "GeometryBuffer.h"

#include <stdexcept>

namespace
{
void AppendVertex(const Geometry& geometry, const GeometryVertex& vertex,
                  PreparedGeometry& prepared)
{
    prepared.vertices.insert(prepared.vertices.end(), {vertex.x, vertex.y, vertex.z});
    if (geometry.hasColors)
        prepared.vertices.insert(prepared.vertices.end(), {vertex.red, vertex.green, vertex.blue});
    if (geometry.hasTextureCoordinates)
        prepared.vertices.insert(prepared.vertices.end(), {vertex.u, vertex.v});
    if (geometry.hasNormals)
        prepared.vertices.insert(prepared.vertices.end(),
                                 {vertex.normalX, vertex.normalY, vertex.normalZ});
}
}

std::size_t PreparedGeometry::VertexCount() const
{
    return layout.strideFloats == 0 ? 0 : vertices.size() / layout.strideFloats;
}

PreparedGeometry PrepareGeometryForGpu(const Geometry& geometry)
{
    PreparedGeometry prepared;
    prepared.layout.hasColors = geometry.hasColors;
    prepared.layout.hasTextureCoordinates = geometry.hasTextureCoordinates;
    prepared.layout.hasNormals = geometry.hasNormals;

    std::size_t offset = 3;
    if (geometry.hasColors)
    {
        prepared.layout.colorOffset = offset;
        offset += 3;
    }
    if (geometry.hasTextureCoordinates)
    {
        prepared.layout.textureCoordinateOffset = offset;
        offset += 2;
    }
    if (geometry.hasNormals)
    {
        prepared.layout.normalOffset = offset;
        offset += 3;
    }
    prepared.layout.strideFloats = offset;

    switch (geometry.topology)
    {
    case PrimitiveTopology::QUADS:
        if (geometry.vertices.size() % 4 != 0)
            throw std::invalid_argument("QUADS geometry must contain complete groups of four vertices");
        prepared.drawMode = GeometryDrawMode::TRIANGLES;
        prepared.vertices.reserve(geometry.vertices.size() / 4 * 6 * offset);
        for (std::size_t i = 0; i < geometry.vertices.size(); i += 4)
        {
            // Preserve the legacy GL_QUADS winding with triangles (0,1,2) and (0,2,3).
            const std::size_t indices[] = {i, i + 1, i + 2, i, i + 2, i + 3};
            for (std::size_t index : indices)
                AppendVertex(geometry, geometry.vertices[index], prepared);
        }
        break;
    case PrimitiveTopology::TRIANGLES:
        if (geometry.vertices.size() % 3 != 0)
            throw std::invalid_argument("TRIANGLES geometry must contain complete groups of three vertices");
        prepared.drawMode = GeometryDrawMode::TRIANGLES;
        break;
    case PrimitiveTopology::LINES:
        if (geometry.vertices.size() % 2 != 0)
            throw std::invalid_argument("LINES geometry must contain complete pairs of vertices");
        prepared.drawMode = GeometryDrawMode::LINES;
        break;
    case PrimitiveTopology::LINE_LOOP:
        prepared.drawMode = GeometryDrawMode::LINE_LOOP;
        break;
    }

    if (geometry.topology != PrimitiveTopology::QUADS)
    {
        prepared.vertices.reserve(geometry.vertices.size() * offset);
        for (const GeometryVertex& vertex : geometry.vertices)
            AppendVertex(geometry, vertex, prepared);
    }
    return prepared;
}
