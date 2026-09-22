#include <gtest/gtest.h>

#include "GeometryBuffer.h"

namespace
{
GeometryVertex Vertex(float x, float y, float z)
{
    return {x, y, z, x + 10, y + 10, 0, 0, 1, x + 20, y + 20, z + 20};
}
}

TEST(GeometryBufferTest, TriangulatesQuadsWithoutChangingWinding)
{
    Geometry geometry;
    geometry.topology = PrimitiveTopology::QUADS;
    geometry.vertices = {Vertex(0, 0, 0), Vertex(1, 0, 0),
                         Vertex(1, 1, 0), Vertex(0, 1, 0)};

    const PreparedGeometry prepared = PrepareGeometryForGpu(geometry);

    EXPECT_EQ(prepared.drawMode, GeometryDrawMode::TRIANGLES);
    ASSERT_EQ(prepared.VertexCount(), 6u);
    const float expectedX[] = {0, 1, 1, 0, 1, 0};
    for (std::size_t i = 0; i < 6; ++i)
        EXPECT_FLOAT_EQ(prepared.vertices[i * 3], expectedX[i]);
}

TEST(GeometryBufferTest, PacksOnlyDeclaredAttributesInDeterministicOrder)
{
    Geometry geometry;
    geometry.topology = PrimitiveTopology::LINES;
    geometry.hasColors = true;
    geometry.hasNormals = true;
    geometry.vertices = {Vertex(1, 2, 3), Vertex(4, 5, 6)};

    const PreparedGeometry prepared = PrepareGeometryForGpu(geometry);

    EXPECT_EQ(prepared.drawMode, GeometryDrawMode::LINES);
    EXPECT_EQ(prepared.layout.strideFloats, 9u);
    EXPECT_EQ(prepared.layout.colorOffset, 3u);
    EXPECT_EQ(prepared.layout.normalOffset, 6u);
    EXPECT_FALSE(prepared.layout.hasTextureCoordinates);
    EXPECT_EQ(prepared.vertices,
              (std::vector<float>{1, 2, 3, 21, 22, 23, 0, 0, 1,
                                  4, 5, 6, 24, 25, 26, 0, 0, 1}));
}

TEST(GeometryBufferTest, PreservesLineLoopAndRejectsIncompletePrimitives)
{
    Geometry loop;
    loop.topology = PrimitiveTopology::LINE_LOOP;
    loop.vertices = {Vertex(0, 0, 0), Vertex(1, 0, 0), Vertex(1, 1, 0)};
    EXPECT_EQ(PrepareGeometryForGpu(loop).drawMode, GeometryDrawMode::LINE_LOOP);

    Geometry incomplete = loop;
    incomplete.topology = PrimitiveTopology::QUADS;
    EXPECT_THROW(PrepareGeometryForGpu(incomplete), std::invalid_argument);
}
