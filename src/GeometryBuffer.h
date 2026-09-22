#pragma once

#include <cstddef>
#include <vector>

#include "Geometry.h"

// WebGL-friendly CPU preparation kept separate from GL so topology conversion
// and the vertex contract can be tested on every platform.
enum class GeometryDrawMode
{
    TRIANGLES,
    LINES,
    LINE_LOOP
};

struct GeometryAttributeLayout
{
    std::size_t strideFloats = 0;
    std::size_t positionOffset = 0;
    std::size_t colorOffset = 0;
    std::size_t textureCoordinateOffset = 0;
    std::size_t normalOffset = 0;
    bool hasColors = false;
    bool hasTextureCoordinates = false;
    bool hasNormals = false;
};

struct PreparedGeometry
{
    GeometryDrawMode drawMode = GeometryDrawMode::TRIANGLES;
    GeometryAttributeLayout layout;
    std::vector<float> vertices;

    std::size_t VertexCount() const;
};

PreparedGeometry PrepareGeometryForGpu(const Geometry& geometry);
