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

Geometry SimpleGeometry::CreateItemFallback()
{
    Geometry geometry;
    // The legacy glScalef call occurred inside the vertex submission block,
    // where OpenGL rejects and ignores it, so the rendered fallback used
    // these unscaled coordinates.
    geometry.vertices = {
        Vertex(-1.0f, -1.0f, 1.0f, 0.0f, 0.0f),
        Vertex(1.0f, -1.0f, 1.0f, 0.0f, 0.0f),
        Vertex(1.0f, 1.0f, 1.0f, 0.0f, 0.0f),
        Vertex(-1.0f, 1.0f, 1.0f, 0.0f, 0.0f)
    };
    return geometry;
}

// Legacy enemy body: preserve triangle order, normals and UVs.
Geometry SimpleGeometry::CreateEnemyBody()
{
    Geometry geometry;
    geometry.topology = PrimitiveTopology::TRIANGLES;
    geometry.hasTextureCoordinates = true;
    geometry.hasNormals = true;
    geometry.vertices = {
        VertexWithNormal(4, 0, -4, 0.000000f, 0.000000f, 0.894427f, -0.447214f, 0.000000f),
        VertexWithNormal(4, 0, 4, 1.000000f, 0.000000f, 0.894427f, -0.447214f, 0.000000f),
        VertexWithNormal(5, 2, 3, 1.000000f, 1.000000f, 0.894427f, -0.447214f, 0.000000f),
        VertexWithNormal(4, 0, -4, 0.000000f, 0.000000f, 0.894427f, -0.447214f, 0.000000f),
        VertexWithNormal(5, 2, 3, 1.000000f, 1.000000f, 0.894427f, -0.447214f, 0.000000f),
        VertexWithNormal(5, 2, -3, 0.000000f, 1.000000f, 0.894427f, -0.447214f, 0.000000f),
        VertexWithNormal(-4, 0, -4, 0.000000f, 0.000000f, -0.894427f, -0.447214f, 0.000000f),
        VertexWithNormal(-5, 2, -5, 0.000000f, 1.000000f, -0.894427f, -0.447214f, 0.000000f),
        VertexWithNormal(-5, 2, 5, 1.000000f, 1.000000f, -0.894427f, -0.447214f, 0.000000f),
        VertexWithNormal(-4, 0, -4, 0.000000f, 0.000000f, -0.894427f, -0.447214f, 0.000000f),
        VertexWithNormal(-5, 2, 5, 1.000000f, 1.000000f, -0.894427f, -0.447214f, 0.000000f),
        VertexWithNormal(-4, 0, 4, 1.000000f, 0.000000f, -0.894427f, -0.447214f, 0.000000f),
        VertexWithNormal(4, 0, -4, 0.000000f, 0.000000f, 0.182574f, 0.365148f, -0.912871f),
        VertexWithNormal(5, 2, -3, 0.000000f, 1.000000f, 0.182574f, 0.365148f, -0.912871f),
        VertexWithNormal(-5, 2, -5, 0.000000f, 1.000000f, 0.182574f, 0.365148f, -0.912871f),
        VertexWithNormal(4, 0, -4, 0.000000f, 0.000000f, 0.000000f, -0.447214f, -0.894427f),
        VertexWithNormal(-5, 2, -5, 0.000000f, 1.000000f, 0.000000f, -0.447214f, -0.894427f),
        VertexWithNormal(-4, 0, -4, 0.000000f, 0.000000f, 0.000000f, -0.447214f, -0.894427f),
        VertexWithNormal(4, 0, 4, 1.000000f, 0.000000f, 0.000000f, -0.447214f, 0.894427f),
        VertexWithNormal(-4, 0, 4, 1.000000f, 0.000000f, 0.000000f, -0.447214f, 0.894427f),
        VertexWithNormal(-5, 2, 5, 1.000000f, 1.000000f, 0.000000f, -0.447214f, 0.894427f),
        VertexWithNormal(4, 0, 4, 1.000000f, 0.000000f, 0.182574f, 0.365148f, 0.912871f),
        VertexWithNormal(-5, 2, 5, 1.000000f, 1.000000f, 0.182574f, 0.365148f, 0.912871f),
        VertexWithNormal(5, 2, 3, 1.000000f, 1.000000f, 0.182574f, 0.365148f, 0.912871f),
        VertexWithNormal(5, 2, -3, 0.000000f, 1.000000f, 0.000000f, 1.000000f, 0.000000f),
        VertexWithNormal(5, 2, 3, 1.000000f, 1.000000f, 0.000000f, 1.000000f, 0.000000f),
        VertexWithNormal(-5, 2, 5, 1.000000f, 1.000000f, 0.000000f, 1.000000f, 0.000000f),
        VertexWithNormal(5, 2, -3, 0.000000f, 1.000000f, 0.000000f, 1.000000f, 0.000000f),
        VertexWithNormal(-5, 2, 5, 1.000000f, 1.000000f, 0.000000f, 1.000000f, 0.000000f),
        VertexWithNormal(-5, 2, -5, 0.000000f, 1.000000f, 0.000000f, 1.000000f, 0.000000f),
        VertexWithNormal(4, 0, -4, 0.000000f, 0.000000f, 0.000000f, -1.000000f, 0.000000f),
        VertexWithNormal(-4, 0, -4, 0.000000f, 0.000000f, 0.000000f, -1.000000f, 0.000000f),
        VertexWithNormal(-4, 0, 4, 1.000000f, 0.000000f, 0.000000f, -1.000000f, 0.000000f),
        VertexWithNormal(4, 0, -4, 0.000000f, 0.000000f, 0.000000f, -1.000000f, 0.000000f),
        VertexWithNormal(-4, 0, 4, 1.000000f, 0.000000f, 0.000000f, -1.000000f, 0.000000f),
        VertexWithNormal(4, 0, 4, 1.000000f, 0.000000f, 0.000000f, -1.000000f, 0.000000f),
    };
    return geometry;
}

// Legacy enemy barrel: preserve triangle order, normals and UVs.
Geometry SimpleGeometry::CreateEnemyBarrel()
{
    Geometry geometry;
    geometry.topology = PrimitiveTopology::TRIANGLES;
    geometry.hasTextureCoordinates = true;
    geometry.hasNormals = true;
    geometry.vertices = {
        VertexWithNormal(3, 2, 2, 0.000000f, 0.000000f, 0.163846f, -0.081923f, 0.983078f),
        VertexWithNormal(-3, 2, 3, 1.000000f, 0.000000f, 0.163846f, -0.081923f, 0.983078f),
        VertexWithNormal(-2, 4, 3, 1.000000f, 1.000000f, 0.163846f, -0.081923f, 0.983078f),
        VertexWithNormal(3, 2, 2, 0.000000f, 0.000000f, 0.180156f, -0.041001f, 0.982783f),
        VertexWithNormal(-2, 4, 3, 1.000000f, 1.000000f, 0.180156f, -0.041001f, 0.982783f),
        VertexWithNormal(3, 4, 2, 0.000000f, 1.000000f, 0.180156f, -0.041001f, 0.982783f),
        VertexWithNormal(3, 2, -2, 0.000000f, 0.000000f, 0.196116f, 0.000000f, -0.980581f),
        VertexWithNormal(3, 4, -2, 0.000000f, 1.000000f, 0.196116f, 0.000000f, -0.980581f),
        VertexWithNormal(-2, 4, -3, 1.000000f, 1.000000f, 0.196116f, 0.000000f, -0.980581f),
        VertexWithNormal(3, 2, -2, 0.000000f, 0.000000f, 0.180156f, -0.041001f, -0.982783f),
        VertexWithNormal(-2, 4, -3, 1.000000f, 1.000000f, 0.180156f, -0.041001f, -0.982783f),
        VertexWithNormal(-3, 2, -3, 1.000000f, 0.000000f, 0.180156f, -0.041001f, -0.982783f),
        VertexWithNormal(3, 2, 2, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f),
        VertexWithNormal(3, 4, 2, 0.000000f, 1.000000f, 1.000000f, 0.000000f, 0.000000f),
        VertexWithNormal(3, 4, -2, 0.000000f, 1.000000f, 1.000000f, 0.000000f, 0.000000f),
        VertexWithNormal(3, 2, 2, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f),
        VertexWithNormal(3, 4, -2, 0.000000f, 1.000000f, 1.000000f, 0.000000f, 0.000000f),
        VertexWithNormal(3, 2, -2, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f),
        VertexWithNormal(-3, 2, 3, 1.000000f, 0.000000f, -0.894427f, 0.447214f, 0.000000f),
        VertexWithNormal(-3, 2, -3, 1.000000f, 0.000000f, -0.894427f, 0.447214f, 0.000000f),
        VertexWithNormal(-2, 4, -3, 1.000000f, 1.000000f, -0.894427f, 0.447214f, 0.000000f),
        VertexWithNormal(-3, 2, 3, 1.000000f, 0.000000f, -0.894427f, 0.447214f, 0.000000f),
        VertexWithNormal(-2, 4, -3, 1.000000f, 1.000000f, -0.894427f, 0.447214f, 0.000000f),
        VertexWithNormal(-2, 4, 3, 1.000000f, 1.000000f, -0.894427f, 0.447214f, 0.000000f),
        VertexWithNormal(3, 4, 2, 0.000000f, 1.000000f, 0.000000f, 1.000000f, 0.000000f),
        VertexWithNormal(-2, 4, 3, 1.000000f, 1.000000f, 0.000000f, 1.000000f, 0.000000f),
        VertexWithNormal(-2, 4, -3, 1.000000f, 1.000000f, 0.000000f, 1.000000f, 0.000000f),
        VertexWithNormal(3, 4, 2, 0.000000f, 1.000000f, 0.000000f, 1.000000f, 0.000000f),
        VertexWithNormal(-2, 4, -3, 1.000000f, 1.000000f, 0.000000f, 1.000000f, 0.000000f),
        VertexWithNormal(3, 4, -2, 0.000000f, 1.000000f, 0.000000f, 1.000000f, 0.000000f),
        VertexWithNormal(3, 2, 2, 0.000000f, 0.000000f, 0.000000f, -1.000000f, 0.000000f),
        VertexWithNormal(3, 2, -2, 0.000000f, 0.000000f, 0.000000f, -1.000000f, 0.000000f),
        VertexWithNormal(-3, 2, -3, 1.000000f, 0.000000f, 0.000000f, -1.000000f, 0.000000f),
        VertexWithNormal(3, 2, 2, 0.000000f, 0.000000f, 0.000000f, -1.000000f, 0.000000f),
        VertexWithNormal(-3, 2, -3, 1.000000f, 0.000000f, 0.000000f, -1.000000f, 0.000000f),
        VertexWithNormal(-3, 2, 3, 1.000000f, 0.000000f, 0.000000f, -1.000000f, 0.000000f),
    };
    return geometry;
}

// Legacy enemy turret: preserve triangle order, normals and UVs.
Geometry SimpleGeometry::CreateEnemyTurret()
{
    Geometry geometry;
    geometry.topology = PrimitiveTopology::TRIANGLES;
    geometry.hasTextureCoordinates = true;
    geometry.hasNormals = true;
    geometry.vertices = {
        VertexWithNormal(-3, 4, -0.25f, 0.000000f, -0.707107f, 0.000000f, 1.000000f, 0.000000f),
        VertexWithNormal(5, 4, -0.25f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f),
        VertexWithNormal(5, 4, 0.25f, 1.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f),
        VertexWithNormal(-3, 4, -0.25f, 1.000000f, 1.000000f, 0.000000f, 1.000000f, 0.000000f),
        VertexWithNormal(5, 4, 0.25f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f),
        VertexWithNormal(-3, 4, 0.25f, 1.000000f, 1.000000f, 0.000000f, 1.000000f, 0.000000f),
        VertexWithNormal(-3, 4, -0.25f, 0.000000f, 1.000000f, 0.000000f, -0.707107f, -0.707107f),
        VertexWithNormal(-3, 3, 0, 0.000000f, 0.000000f, 0.000000f, -0.707107f, -0.707107f),
        VertexWithNormal(5, 3, 0, 0.000000f, 1.000000f, 0.000000f, -0.707107f, -0.707107f),
        VertexWithNormal(-3, 4, -0.25f, 1.000000f, 1.000000f, 0.000000f, -0.707107f, -0.707107f),
        VertexWithNormal(5, 3, 0, 0.000000f, 0.000000f, 0.000000f, -0.707107f, -0.707107f),
        VertexWithNormal(5, 4, -0.25f, 1.000000f, 1.000000f, 0.000000f, -0.707107f, -0.707107f),
        VertexWithNormal(-3, 4, -0.25f, 1.000000f, 0.000000f, -1.000000f, 0.000000f, 0.000000f),
        VertexWithNormal(-3, 4, 0.25f, 0.000000f, 0.000000f, -1.000000f, 0.000000f, 0.000000f),
        VertexWithNormal(-3, 3, 0, 0.000000f, 1.000000f, -1.000000f, 0.000000f, 0.000000f),
        VertexWithNormal(5, 4, -0.25f, 0.000000f, 1.000000f, 1.000000f, 0.000000f, 0.000000f),
        VertexWithNormal(5, 3, 0, 1.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f),
        VertexWithNormal(5, 4, 0.25f, 1.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f),
        VertexWithNormal(-3, 4, 0.25f, 1.000000f, 1.000000f, 0.000000f, -0.707107f, 0.707107f),
        VertexWithNormal(5, 4, 0.25f, 0.000000f, 1.000000f, 0.000000f, -0.707107f, 0.707107f),
        VertexWithNormal(5, 3, 0, 1.000000f, 1.000000f, 0.000000f, -0.707107f, 0.707107f),
        VertexWithNormal(-3, 4, 0.25f, 1.000000f, 1.000000f, 0.000000f, -0.707107f, 0.707107f),
        VertexWithNormal(5, 3, 0, 0.000000f, 1.000000f, 0.000000f, -0.707107f, 0.707107f),
        VertexWithNormal(-3, 3, 0, 1.000000f, 1.000000f, 0.000000f, -0.707107f, 0.707107f),
    };
    return geometry;
}
