#pragma once

#include <vector>

enum class PrimitiveTopology
{
    QUADS,
    LINE_LOOP,
    TRIANGLES
};

struct GeometryVertex
{
    float x;
    float y;
    float z;
    float u;
    float v;
    float normalX;
    float normalY;
    float normalZ;
    float red;
    float green;
    float blue;
};

struct Geometry
{
    PrimitiveTopology topology = PrimitiveTopology::QUADS;
    std::vector<GeometryVertex> vertices;
    bool hasTextureCoordinates = false;
    bool hasNormals = false;
    bool hasColors = false;
};

namespace SimpleGeometry
{
Geometry CreateCube(float halfExtent = 0.5f);
Geometry CreateHorizontalSquare(float halfExtent = 0.5f,
                                PrimitiveTopology topology = PrimitiveTopology::QUADS);
Geometry CreateVerticalSquare(float halfExtent = 0.5f);
Geometry CreateBullet();
Geometry CreateItemFallback();
}
