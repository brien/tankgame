#include "Geometry.h"

namespace
{
GeometryVertex Vertex(float x, float y, float z, float u, float v)
{
    return {x, y, z, u, v, 0.0f, 0.0f, 0.0f};
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
