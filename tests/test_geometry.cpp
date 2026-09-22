#include <gtest/gtest.h>

#include "Geometry.h"
#include "igtl_qmesh.h"

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

TEST(GeometryTest, BulletPreservesTriangleAttributesAndLegacyOrdering)
{
    const Geometry bullet = SimpleGeometry::CreateBullet();

    ASSERT_EQ(bullet.topology, PrimitiveTopology::TRIANGLES);
    ASSERT_EQ(bullet.vertices.size(), 24u);
    EXPECT_TRUE(bullet.hasTextureCoordinates);
    EXPECT_TRUE(bullet.hasNormals);

    const GeometryVertex& topFrontLeft = bullet.vertices[0];
    EXPECT_FLOAT_EQ(topFrontLeft.x, -0.3f);
    EXPECT_FLOAT_EQ(topFrontLeft.y, 0.1f);
    EXPECT_FLOAT_EQ(topFrontLeft.z, -0.025f);
    EXPECT_FLOAT_EQ(topFrontLeft.u, 0.0f);
    EXPECT_FLOAT_EQ(topFrontLeft.v, -0.707107f);
    EXPECT_FLOAT_EQ(topFrontLeft.normalX, 0.0f);
    EXPECT_FLOAT_EQ(topFrontLeft.normalY, 1.0f);
    EXPECT_FLOAT_EQ(topFrontLeft.normalZ, 0.0f);

    const GeometryVertex& lowerRidge = bullet.vertices[7];
    EXPECT_FLOAT_EQ(lowerRidge.x, 0.0f);
    EXPECT_FLOAT_EQ(lowerRidge.y, 0.0f);
    EXPECT_FLOAT_EQ(lowerRidge.z, 0.0f);
    EXPECT_FLOAT_EQ(lowerRidge.normalY, -0.707107f);
    EXPECT_FLOAT_EQ(lowerRidge.normalZ, -0.707107f);

    const GeometryVertex& rightFace = bullet.vertices[15];
    EXPECT_FLOAT_EQ(rightFace.normalX, 1.0f);
    EXPECT_FLOAT_EQ(rightFace.normalY, 0.0f);
    EXPECT_FLOAT_EQ(rightFace.normalZ, 0.0f);

    EXPECT_FLOAT_EQ(bullet.vertices[18].normalZ, 0.707107f);
    EXPECT_FLOAT_EQ(bullet.vertices[18].x, bullet.vertices[21].x);
    EXPECT_FLOAT_EQ(bullet.vertices[18].y, bullet.vertices[21].y);
    EXPECT_FLOAT_EQ(bullet.vertices[18].z, bullet.vertices[21].z);
}

TEST(GeometryTest, ItemFallbackPreservesLegacyQuad)
{
    const Geometry item = SimpleGeometry::CreateItemFallback();

    ASSERT_EQ(item.topology, PrimitiveTopology::QUADS);
    ASSERT_EQ(item.vertices.size(), 4u);
    EXPECT_FALSE(item.hasTextureCoordinates);
    EXPECT_FALSE(item.hasNormals);

    EXPECT_FLOAT_EQ(item.vertices[0].x, -1.0f);
    EXPECT_FLOAT_EQ(item.vertices[0].y, -1.0f);
    EXPECT_FLOAT_EQ(item.vertices[0].z, 1.0f);
    EXPECT_FLOAT_EQ(item.vertices[1].x, 1.0f);
    EXPECT_FLOAT_EQ(item.vertices[2].y, 1.0f);
    EXPECT_FLOAT_EQ(item.vertices[3].x, -1.0f);
}

TEST(GeometryTest, MeshTriangleExtractionPreservesVertexAttributesAndOrder)
{
    igtl_QGLMesh mesh;
    igtl_QGLVertex vertices[] = {
        {1.0f, 2.0f, 3.0f, 0.1f, 0.2f, 0.3f, 0.25f, 0.5f},
        {4.0f, 5.0f, 6.0f, 0.4f, 0.5f, 0.6f, 0.75f, 1.0f},
        {7.0f, 8.0f, 9.0f, 0.7f, 0.8f, 0.9f, 0.0f, 0.25f}
    };
    for (igtl_QGLVertex& vertex : vertices)
        mesh.AddVertex(vertex);

    igtl_QGLTriangle first = {2, 0, 1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0};
    igtl_QGLTriangle second = {2, 1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0};
    mesh.AddTriangle(first);
    mesh.AddTriangle(second);

    const Geometry geometry = mesh.CreateTriangleGeometry();

    ASSERT_EQ(geometry.topology, PrimitiveTopology::TRIANGLES);
    ASSERT_EQ(geometry.vertices.size(), 6u);
    EXPECT_TRUE(geometry.hasNormals);
    EXPECT_TRUE(geometry.hasTextureCoordinates);

    EXPECT_FLOAT_EQ(geometry.vertices[0].x, 7.0f);
    EXPECT_FLOAT_EQ(geometry.vertices[1].x, 1.0f);
    EXPECT_FLOAT_EQ(geometry.vertices[2].x, 4.0f);
    EXPECT_FLOAT_EQ(geometry.vertices[3].x, 7.0f);
    EXPECT_FLOAT_EQ(geometry.vertices[0].normalX, 0.7f);
    EXPECT_FLOAT_EQ(geometry.vertices[0].normalY, 0.8f);
    EXPECT_FLOAT_EQ(geometry.vertices[0].normalZ, 0.9f);
    EXPECT_FLOAT_EQ(geometry.vertices[0].u, 0.0f);
    EXPECT_FLOAT_EQ(geometry.vertices[0].v, 0.25f);
}

TEST(GeometryTest, MeshTriangleExtrusionPreservesLegacyFaceOffsetContract)
{
    igtl_QGLMesh mesh;
    igtl_QGLVertex vertices[] = {
        {1.0f, 2.0f, 3.0f, 0.1f, 0.2f, 0.3f, 0.25f, 0.5f},
        {4.0f, 5.0f, 6.0f, 0.4f, 0.5f, 0.6f, 0.75f, 1.0f},
        {7.0f, 8.0f, 9.0f, 0.7f, 0.8f, 0.9f, 0.0f, 0.25f}
    };
    for (igtl_QGLVertex& vertex : vertices)
        mesh.AddVertex(vertex);

    igtl_QGLTriangle triangle = {2, 0, 1, 2.0f, -3.0f, 4.0f,
                                 0.0f, 0.0f, 0.0f, 0};
    mesh.AddTriangle(triangle);
    igtl_QGLTriangle second = {2, 1, 0, -2.0f, 1.0f, 0.5f,
                               0.0f, 0.0f, 0.0f, 0};
    mesh.AddTriangle(second);

    const Geometry geometry = mesh.CreateTriangleExtrudedGeometry(0.5f);

    ASSERT_EQ(geometry.topology, PrimitiveTopology::TRIANGLES);
    ASSERT_EQ(geometry.vertices.size(), 6u);
    EXPECT_TRUE(geometry.hasNormals);
    EXPECT_TRUE(geometry.hasTextureCoordinates);

    // Winding and vertex duplication match the source triangle exactly; the
    // legacy path emits no back face or side faces.
    EXPECT_FLOAT_EQ(geometry.vertices[0].x, 8.0f);
    EXPECT_FLOAT_EQ(geometry.vertices[0].y, 6.5f);
    EXPECT_FLOAT_EQ(geometry.vertices[0].z, 11.0f);
    EXPECT_FLOAT_EQ(geometry.vertices[1].x, 2.0f);
    EXPECT_FLOAT_EQ(geometry.vertices[1].y, 0.5f);
    EXPECT_FLOAT_EQ(geometry.vertices[1].z, 5.0f);
    EXPECT_FLOAT_EQ(geometry.vertices[2].x, 5.0f);
    EXPECT_FLOAT_EQ(geometry.vertices[2].y, 3.5f);
    EXPECT_FLOAT_EQ(geometry.vertices[2].z, 8.0f);

    // A shared source vertex is emitted again and offset by the second face's
    // facet normal rather than shared or welded in the output.
    EXPECT_FLOAT_EQ(geometry.vertices[3].x, 6.0f);
    EXPECT_FLOAT_EQ(geometry.vertices[3].y, 8.5f);
    EXPECT_FLOAT_EQ(geometry.vertices[3].z, 9.25f);

    // Facet normals control positions, while submitted normals and UVs remain
    // the original per-vertex attributes.
    EXPECT_FLOAT_EQ(geometry.vertices[0].normalX, 0.7f);
    EXPECT_FLOAT_EQ(geometry.vertices[0].normalY, 0.8f);
    EXPECT_FLOAT_EQ(geometry.vertices[0].normalZ, 0.9f);
    EXPECT_FLOAT_EQ(geometry.vertices[0].u, 0.0f);
    EXPECT_FLOAT_EQ(geometry.vertices[0].v, 0.25f);
}

TEST(GeometryTest, ColoredMeshTrianglesPreservePerFaceColorsAndVertexOrder)
{
    igtl_QGLMesh mesh;
    igtl_QGLVertex vertices[] = {
        {1.0f, 2.0f, 3.0f, 0.1f, 0.2f, 0.3f, 0.25f, 0.5f},
        {4.0f, 5.0f, 6.0f, 0.4f, 0.5f, 0.6f, 0.75f, 1.0f},
        {7.0f, 8.0f, 9.0f, 0.7f, 0.8f, 0.9f, 0.0f, 0.25f}
    };
    for (igtl_QGLVertex& vertex : vertices)
        mesh.AddVertex(vertex);

    igtl_QGLTriangle first = {2, 0, 1, 0.0f, 0.0f, 1.0f,
                              0.125f, 0.25f, 0.5f, 3};
    igtl_QGLTriangle second = {2, 1, 0, 0.0f, 1.0f, 0.0f,
                               0.75f, 0.625f, 0.375f, 7};
    mesh.AddTriangle(first);
    mesh.AddTriangle(second);

    const Geometry geometry = mesh.CreateTriangleColoredGeometry();

    ASSERT_EQ(geometry.topology, PrimitiveTopology::TRIANGLES);
    ASSERT_EQ(geometry.vertices.size(), 6u);
    EXPECT_TRUE(geometry.hasNormals);
    EXPECT_TRUE(geometry.hasTextureCoordinates);
    EXPECT_TRUE(geometry.hasColors);

    // Triangle winding and shared-source duplication are unchanged.
    EXPECT_FLOAT_EQ(geometry.vertices[0].x, 7.0f);
    EXPECT_FLOAT_EQ(geometry.vertices[1].x, 1.0f);
    EXPECT_FLOAT_EQ(geometry.vertices[2].x, 4.0f);
    EXPECT_FLOAT_EQ(geometry.vertices[3].x, 7.0f);
    EXPECT_FLOAT_EQ(geometry.vertices[4].x, 4.0f);
    EXPECT_FLOAT_EQ(geometry.vertices[5].x, 1.0f);

    EXPECT_FLOAT_EQ(geometry.vertices[0].normalX, 0.7f);
    EXPECT_FLOAT_EQ(geometry.vertices[0].normalY, 0.8f);
    EXPECT_FLOAT_EQ(geometry.vertices[0].normalZ, 0.9f);
    EXPECT_FLOAT_EQ(geometry.vertices[0].u, 0.0f);
    EXPECT_FLOAT_EQ(geometry.vertices[0].v, 0.25f);

    for (size_t i = 0; i < 3; ++i) {
        EXPECT_FLOAT_EQ(geometry.vertices[i].red, 0.125f);
        EXPECT_FLOAT_EQ(geometry.vertices[i].green, 0.25f);
        EXPECT_FLOAT_EQ(geometry.vertices[i].blue, 0.5f);
    }
    for (size_t i = 3; i < 6; ++i) {
        EXPECT_FLOAT_EQ(geometry.vertices[i].red, 0.75f);
        EXPECT_FLOAT_EQ(geometry.vertices[i].green, 0.625f);
        EXPECT_FLOAT_EQ(geometry.vertices[i].blue, 0.375f);
    }
}

TEST(GeometryTest, ColoredExtrudedTrianglesCombineFaceOffsetAndColor)
{
    igtl_QGLMesh mesh;
    igtl_QGLVertex vertices[] = {
        {1, 2, 3, .1f, .2f, .3f, .25f, .5f},
        {4, 5, 6, .4f, .5f, .6f, .75f, 1},
        {7, 8, 9, .7f, .8f, .9f, 0, .25f}
    };
    for (igtl_QGLVertex& vertex : vertices)
        mesh.AddVertex(vertex);
    igtl_QGLTriangle triangle = {2, 0, 1, 2, -3, 4, .125f, .25f, .5f, 3};
    mesh.AddTriangle(triangle);

    const Geometry geometry = mesh.CreateTriangleColoredExtrudedGeometry(.5f);

    ASSERT_EQ(geometry.topology, PrimitiveTopology::TRIANGLES);
    ASSERT_EQ(geometry.vertices.size(), 3u);
    EXPECT_TRUE(geometry.hasNormals);
    EXPECT_TRUE(geometry.hasTextureCoordinates);
    EXPECT_TRUE(geometry.hasColors);
    EXPECT_FLOAT_EQ(geometry.vertices[0].x, 8);
    EXPECT_FLOAT_EQ(geometry.vertices[0].y, 6.5f);
    EXPECT_FLOAT_EQ(geometry.vertices[0].z, 11);
    EXPECT_FLOAT_EQ(geometry.vertices[0].normalX, .7f);
    EXPECT_FLOAT_EQ(geometry.vertices[0].u, 0);
    for (const GeometryVertex& vertex : geometry.vertices) {
        EXPECT_FLOAT_EQ(vertex.red, .125f);
        EXPECT_FLOAT_EQ(vertex.green, .25f);
        EXPECT_FLOAT_EQ(vertex.blue, .5f);
    }
}

TEST(GeometryTest, MeshEdgesPreserveFlagsOrderingAndVertexAttributes)
{
    igtl_QGLMesh mesh;
    igtl_QGLVertex vertices[] = {
        {1, 2, 3, .1f, .2f, .3f, .25f, .5f},
        {4, 5, 6, .4f, .5f, .6f, .75f, 1},
        {7, 8, 9, .7f, .8f, .9f, 0, .25f}
    };
    for (igtl_QGLVertex& vertex : vertices)
        mesh.AddVertex(vertex);
    igtl_QGLEdge disabled = {0, 2, 9, 8, 7, .9f, .8f, .7f, 0};
    igtl_QGLEdge first = {2, 0, 2, -3, 4, .125f, .25f, .5f, 1};
    igtl_QGLEdge second = {1, 2, -2, 1, .5f, .75f, .625f, .375f, 1};
    mesh.AddEdge(disabled);
    mesh.AddEdge(first);
    mesh.AddEdge(second);

    const Geometry geometry = mesh.CreateEdgeGeometry();

    ASSERT_EQ(geometry.topology, PrimitiveTopology::LINES);
    ASSERT_EQ(geometry.vertices.size(), 4u);
    EXPECT_TRUE(geometry.hasNormals);
    EXPECT_TRUE(geometry.hasTextureCoordinates);
    EXPECT_FALSE(geometry.hasColors);
    EXPECT_FLOAT_EQ(geometry.vertices[0].x, 7);
    EXPECT_FLOAT_EQ(geometry.vertices[1].x, 1);
    EXPECT_FLOAT_EQ(geometry.vertices[2].x, 4);
    EXPECT_FLOAT_EQ(geometry.vertices[3].x, 7);
    EXPECT_FLOAT_EQ(geometry.vertices[0].normalZ, .9f);
    EXPECT_FLOAT_EQ(geometry.vertices[0].u, 0);
    EXPECT_FLOAT_EQ(geometry.vertices[1].v, .5f);
}

TEST(GeometryTest, ExtrudedAndColoredEdgesPreserveEdgeNormalOffsetAndColor)
{
    igtl_QGLMesh mesh;
    igtl_QGLVertex firstVertex = {1, 2, 3, .1f, .2f, .3f, .25f, .5f};
    igtl_QGLVertex secondVertex = {4, 5, 6, .4f, .5f, .6f, .75f, 1};
    mesh.AddVertex(firstVertex);
    mesh.AddVertex(secondVertex);
    igtl_QGLEdge edge = {1, 0, 2, -3, 4, .125f, .25f, .5f, 1};
    mesh.AddEdge(edge);

    const Geometry extruded = mesh.CreateEdgeExtrudedGeometry(.5f);
    const Geometry colored = mesh.CreateEdgeColoredGeometry();
    const Geometry combined = mesh.CreateEdgeColoredExtrudedGeometry(.5f);

    ASSERT_EQ(combined.vertices.size(), 2u);
    EXPECT_FLOAT_EQ(combined.vertices[0].x, 5);
    EXPECT_FLOAT_EQ(combined.vertices[0].y, 3.5f);
    EXPECT_FLOAT_EQ(combined.vertices[0].z, 8);
    EXPECT_FLOAT_EQ(combined.vertices[1].x, 2);
    EXPECT_FLOAT_EQ(combined.vertices[1].y, .5f);
    EXPECT_FLOAT_EQ(combined.vertices[1].z, 5);
    EXPECT_FLOAT_EQ(combined.vertices[0].red, .125f);
    EXPECT_FLOAT_EQ(combined.vertices[0].green, .25f);
    EXPECT_FLOAT_EQ(combined.vertices[0].blue, .5f);
    EXPECT_TRUE(combined.hasColors);
    EXPECT_FALSE(extruded.hasColors);
    EXPECT_FLOAT_EQ(extruded.vertices[0].x, combined.vertices[0].x);
    EXPECT_TRUE(colored.hasColors);
    EXPECT_FLOAT_EQ(colored.vertices[0].x, 4);
    EXPECT_FLOAT_EQ(colored.vertices[0].y, 5);
    EXPECT_FLOAT_EQ(colored.vertices[0].z, 6);
}
