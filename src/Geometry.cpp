#include "Geometry.h"

namespace
{
GeometryVertex Vertex(float x, float y, float z, float u, float v)
{
    return {x, y, z, u, v, 0.0f, 0.0f, 0.0f};
}

GeometryVertex VertexWithNormal(float x, float y, float z, float u, float v,
                                float normalX, float normalY, float normalZ)
{
    return {x, y, z, u, v, normalX, normalY, normalZ};
}
}

Geometry SimpleGeometry::CreateCube(float h)
{
    Geometry geometry;
    geometry.hasTextureCoordinates = true;
    geometry.vertices = {
        Vertex(-h, h, -h, 0, 1), Vertex(-h, h, h, 0, 0),
        Vertex(h, h, h, 1, 0), Vertex(h, h, -h, 1, 1),
        Vertex(-h, -h, -h, 1, 1), Vertex(h, -h, -h, 0, 1),
        Vertex(h, -h, h, 0, 0), Vertex(-h, -h, h, 1, 0),
        Vertex(-h, -h, h, 0, 0), Vertex(h, -h, h, 1, 0),
        Vertex(h, h, h, 1, 1), Vertex(-h, h, h, 0, 1),
        Vertex(-h, -h, -h, 1, 0), Vertex(-h, h, -h, 1, 1),
        Vertex(h, h, -h, 0, 1), Vertex(h, -h, -h, 0, 0),
        Vertex(h, -h, -h, 1, 0), Vertex(h, h, -h, 1, 1),
        Vertex(h, h, h, 0, 1), Vertex(h, -h, h, 0, 0),
        Vertex(-h, -h, -h, 0, 0), Vertex(-h, -h, h, 1, 0),
        Vertex(-h, h, h, 1, 1), Vertex(-h, h, -h, 0, 1)
    };
    return geometry;
}

Geometry SimpleGeometry::CreateHorizontalSquare(float h, PrimitiveTopology topology)
{
    Geometry geometry;
    geometry.topology = topology;
    geometry.hasTextureCoordinates = true;
    geometry.vertices = {
        Vertex(-h, 0, -h, 1, 1), Vertex(h, 0, -h, 0, 1),
        Vertex(h, 0, h, 0, 0), Vertex(-h, 0, h, 1, 0)
    };
    return geometry;
}

Geometry SimpleGeometry::CreateVerticalSquare(float h)
{
    Geometry geometry;
    geometry.hasTextureCoordinates = true;
    geometry.vertices = {
        Vertex(-h, -h, 0, 0, 0), Vertex(h, -h, 0, 1, 0),
        Vertex(h, h, 0, 1, 1), Vertex(-h, h, 0, 0, 1)
    };
    return geometry;
}

Geometry SimpleGeometry::CreateBullet()
{
    Geometry geometry;
    geometry.topology = PrimitiveTopology::TRIANGLES;
    geometry.hasTextureCoordinates = true;
    geometry.hasNormals = true;

    const float diagonal = 0.707107f;
    geometry.vertices = {
        VertexWithNormal(-0.3f, 0.1f, -0.025f, 0.0f, -diagonal, 0.0f, 1.0f, 0.0f),
        VertexWithNormal(0.5f, 0.1f, -0.025f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f),
        VertexWithNormal(0.5f, 0.1f, 0.025f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
        VertexWithNormal(-0.3f, 0.1f, -0.025f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f),
        VertexWithNormal(0.5f, 0.1f, 0.025f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f),
        VertexWithNormal(-0.3f, 0.1f, 0.025f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f),

        VertexWithNormal(-0.3f, 0.1f, -0.025f, 0.0f, 1.0f, 0.0f, -diagonal, -diagonal),
        VertexWithNormal(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -diagonal, -diagonal),
        VertexWithNormal(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -diagonal, -diagonal),
        VertexWithNormal(-0.3f, 0.1f, -0.025f, 1.0f, 1.0f, 0.0f, -diagonal, -diagonal),
        VertexWithNormal(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -diagonal, -diagonal),
        VertexWithNormal(0.5f, 0.1f, -0.025f, 1.0f, 1.0f, 0.0f, -diagonal, -diagonal),

        VertexWithNormal(-0.3f, 0.1f, -0.025f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f),
        VertexWithNormal(-0.3f, 0.1f, 0.025f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
        VertexWithNormal(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f),
        VertexWithNormal(0.5f, 0.1f, -0.025f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f),
        VertexWithNormal(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f),
        VertexWithNormal(0.5f, 0.1f, 0.025f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f),

        VertexWithNormal(-0.3f, 0.1f, 0.025f, 1.0f, 1.0f, 0.0f, -diagonal, diagonal),
        VertexWithNormal(0.5f, 0.1f, 0.025f, 0.0f, 1.0f, 0.0f, -diagonal, diagonal),
        VertexWithNormal(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, -diagonal, diagonal),
        VertexWithNormal(-0.3f, 0.1f, 0.025f, 1.0f, 1.0f, 0.0f, -diagonal, diagonal),
        VertexWithNormal(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -diagonal, diagonal),
        VertexWithNormal(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, -diagonal, diagonal)
    };
    return geometry;
}
