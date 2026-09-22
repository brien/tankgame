#include <gtest/gtest.h>

#include "Geometry.h"

TEST(GeometryTest, CubeHasSixTexturedQuadFaces)
{
    const Geometry cube = SimpleGeometry::CreateCube();

    EXPECT_EQ(cube.topology, PrimitiveTopology::QUADS);
    EXPECT_EQ(cube.vertices.size(), 24u);
    EXPECT_TRUE(cube.hasTextureCoordinates);
    EXPECT_FALSE(cube.hasNormals);

    bool foundTopFrontLeft = false;
    bool foundBottomBackRight = false;
    for (const GeometryVertex& vertex : cube.vertices)
    {
        foundTopFrontLeft |= vertex.x == -0.5f && vertex.y == 0.5f && vertex.z == 0.5f;
        foundBottomBackRight |= vertex.x == 0.5f && vertex.y == -0.5f && vertex.z == -0.5f;
    }
    EXPECT_TRUE(foundTopFrontLeft);
    EXPECT_TRUE(foundBottomBackRight);
}

TEST(GeometryTest, HorizontalSquareCanDescribeFilledAndOutlineResources)
{
    const Geometry filled = SimpleGeometry::CreateHorizontalSquare();
    const Geometry outline = SimpleGeometry::CreateHorizontalSquare(
        0.5f, PrimitiveTopology::LINE_LOOP);

    EXPECT_EQ(filled.topology, PrimitiveTopology::QUADS);
    EXPECT_EQ(outline.topology, PrimitiveTopology::LINE_LOOP);
    EXPECT_EQ(filled.vertices.size(), 4u);
    EXPECT_EQ(outline.vertices.size(), 4u);
    EXPECT_TRUE(filled.hasTextureCoordinates);
    for (const GeometryVertex& vertex : filled.vertices)
        EXPECT_FLOAT_EQ(vertex.y, 0.0f);
}

TEST(GeometryTest, PrimitiveScaleIsAppliedToCpuPositions)
{
    const Geometry cube = SimpleGeometry::CreateCube(0.05f);
    const Geometry square = SimpleGeometry::CreateVerticalSquare(1.0f);

    EXPECT_FLOAT_EQ(cube.vertices.front().x, -0.05f);
    EXPECT_FLOAT_EQ(cube.vertices.front().y, 0.05f);
    EXPECT_FLOAT_EQ(square.vertices.front().x, -1.0f);
    EXPECT_FLOAT_EQ(square.vertices.front().z, 0.0f);
}
