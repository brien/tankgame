#include <gtest/gtest.h>
#include "rendering/TerrainGeometry.h"

namespace
{
const Geometry& Batch(const TerrainBatches& batches, TerrainBatch batch)
{
    return batches[static_cast<std::size_t>(batch)];
}

TerrainRenderData SmallTerrain(int level = 48)
{
    TerrainRenderData t;
    t.sizeX = t.sizeZ = 5;
    t.levelNumber = level;
    t.colors.defaultColor = Vector3(.2f, .3f, .4f);
    t.colors.blockColor = Vector3(.6f, .7f, .8f);
    return t;
}

float NormalY(const Geometry& geometry)
{
    const auto& a = geometry.vertices[0];
    const auto& b = geometry.vertices[1];
    const auto& c = geometry.vertices[2];
    return (b.z-a.z)*(c.x-a.x) - (b.x-a.x)*(c.z-a.z);
}
}

TEST(TerrainGeometryTest, FlatSurfacePreservesLegacyStripEndpointsUVsAndClockwiseWinding)
{
    auto t = SmallTerrain();
    const auto batches = TerrainGeometry::Build(t);
    const auto& g = Batch(batches, TerrainBatch::BlackSurface);
    ASSERT_EQ(g.vertices.size(), 16u);
    EXPECT_TRUE(g.hasTextureCoordinates);
    EXPECT_TRUE(g.hasColors);
    EXPECT_EQ(g.vertices[0].x, 0);
    EXPECT_EQ(g.vertices[0].z, -1); // Historical last-strip extension.
    EXPECT_EQ(g.vertices[0].u, 5);
    EXPECT_EQ(g.vertices[0].v, 1);
    EXPECT_EQ(g.vertices[2].x, 1);
    EXPECT_EQ(g.vertices[2].z, 4);
    EXPECT_EQ(g.vertices[2].u, 0);
    EXPECT_EQ(g.vertices[2].v, 0);
    EXPECT_LT(NormalY(g), 0); // CW viewed from above.
    EXPECT_FLOAT_EQ(g.vertices[0].red, .6f);
    EXPECT_TRUE(Batch(batches, TerrainBatch::Walls).vertices.empty());
}

TEST(TerrainGeometryTest, HeightAndLevelSelectExistingSurfaceTextures)
{
    for (int level : {0, 48, 49, 50, 56, 57, 58, 69, 70, 71}) {
        auto t = SmallTerrain(level);
        for (int x = 0; x < t.sizeX; ++x)
            for (int z = 0; z < t.sizeZ; ++z) t.heightMap[x][z] = 2;
        const auto batches = TerrainGeometry::Build(t);
        const bool checker = level == 0 || level == 50 || level == 56 || level == 57 || level == 58;
        const bool white = level == 48 || level == 69 || level == 70;
        EXPECT_EQ(Batch(batches, TerrainBatch::CheckerSurface).vertices.size(), checker ? 16u : 0u);
        EXPECT_EQ(Batch(batches, TerrainBatch::WhiteSurface).vertices.size(), white ? 16u : 0u);
        EXPECT_EQ(Batch(batches, TerrainBatch::BlackSurface).vertices.size(), !checker && !white ? 16u : 0u);
        const auto& g = Batch(batches, checker ? TerrainBatch::CheckerSurface :
                             white ? TerrainBatch::WhiteSurface : TerrainBatch::BlackSurface);
        EXPECT_FLOAT_EQ(g.vertices[0].red, .2f);
        EXPECT_EQ(g.vertices[0].y, 2);
    }
}

TEST(TerrainGeometryTest, HeightCutoffAndNegativeFinalStripMatchCompatibility)
{
    auto t = SmallTerrain();
    for (int x = 0; x < t.sizeX; ++x)
        for (int z = 0; z < t.sizeZ; ++z) t.heightMap[x][z] = 25;
    auto batches = TerrainGeometry::Build(t);
    EXPECT_TRUE(Batch(batches, TerrainBatch::WhiteSurface).vertices.empty());
    t.heightMap[0][0] = -2; t.heightMap[0][1] = 0;
    t.heightMap[1][0] = -2; t.heightMap[1][1] = -2; t.heightMap[1][2] = -2;
    batches = TerrainGeometry::Build(t);
    const auto& g = Batch(batches, TerrainBatch::WhiteSurface);
    ASSERT_EQ(g.vertices.size(), 4u); // Only the negative *final* strip survives.
    EXPECT_EQ(g.vertices[0].x, 1);
    EXPECT_EQ(g.vertices[0].y, -2);
}

TEST(TerrainGeometryTest, IsolatedStepEmitsFourOutwardClockwiseWallsWithRepeatingUVs)
{
    auto t = SmallTerrain();
    t.heightMap[2][2] = 3;
    const auto batches = TerrainGeometry::Build(t);
    const auto& g = Batch(batches, TerrainBatch::Walls);
    ASSERT_EQ(g.vertices.size(), 16u);
    for (std::size_t i = 0; i < g.vertices.size(); i += 4) {
        const auto& a = g.vertices[i]; const auto& b = g.vertices[i+1]; const auto& c = g.vertices[i+2];
        const float nx = (b.y-a.y)*(c.z-a.z)-(b.z-a.z)*(c.y-a.y);
        const float nz = (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);
        const float cx = (a.x+b.x+c.x+g.vertices[i+3].x)/4 - 2.5f;
        const float cz = (a.z+b.z+c.z+g.vertices[i+3].z)/4 - 2.5f;
        EXPECT_LT(nx*cx+nz*cz, 0); // CW cross product points into the cell.
        EXPECT_EQ(c.v, 3);
        EXPECT_FLOAT_EQ(a.red, .2f);
    }
}

TEST(TerrainGeometryTest, WallsRisingFromNegativeHeightUseBlockColor)
{
    auto t = SmallTerrain();
    t.heightMap[2][1] = -2; t.heightMap[2][2] = 3;
    const auto batches = TerrainGeometry::Build(t);
    const auto& g = Batch(batches, TerrainBatch::Walls);
    ASSERT_GE(g.vertices.size(), 4u);
    EXPECT_EQ(g.vertices[0].y, -2);
    EXPECT_EQ(g.vertices[2].v, 5);
    EXPECT_FLOAT_EQ(g.vertices[0].red, .6f);
}

TEST(TerrainGeometryTest, FloatingCubeRetainsBoundsUVsColorAndCounterClockwiseWinding)
{
    auto t = SmallTerrain(); t.floatMap[2][3] = 7;
    const auto batches = TerrainGeometry::Build(t);
    const auto& g = Batch(batches, TerrainBatch::Floating);
    ASSERT_EQ(g.vertices.size(), 24u);
    EXPECT_GT(NormalY(g), 0);
    for (const auto& v : g.vertices) {
        EXPECT_GE(v.x, 2); EXPECT_LE(v.x, 3);
        EXPECT_GE(v.y, 6); EXPECT_LE(v.y, 7);
        EXPECT_GE(v.z, 3); EXPECT_LE(v.z, 4);
        EXPECT_GE(v.u, 0); EXPECT_LE(v.u, 1);
        EXPECT_FLOAT_EQ(v.green, .7f);
    }
}

TEST(TerrainGeometryTest, BoundaryRetainsCeilingRectangularCoordinatesAndUndergroundTranslation)
{
    auto t = SmallTerrain(50); t.sizeZ = 7;
    auto batches = TerrainGeometry::Build(t);
    const auto& above = Batch(batches, TerrainBatch::Boundary);
    ASSERT_EQ(above.vertices.size(), 20u);
    EXPECT_EQ(above.vertices[0].y, 26);
    EXPECT_EQ(above.vertices[2].x, 6); // Legacy sz, not sx.
    EXPECT_EQ(above.vertices[4].y, 30);
    EXPECT_EQ(above.vertices[6].y, 0);
    EXPECT_FLOAT_EQ(above.vertices[4].red, .2f);
    EXPECT_FLOAT_EQ(above.vertices[6].red, .6f);
    t.levelNumber = 48;
    batches = TerrainGeometry::Build(t);
    const auto& below = Batch(batches, TerrainBatch::Boundary);
    ASSERT_EQ(below.vertices.size(), 16u);
    EXPECT_EQ(below.vertices[0].y, 0);
    EXPECT_EQ(below.vertices[2].y, -30);
    EXPECT_EQ(below.vertices[0].v, 30);
}

TEST(TerrainGeometryTest, CacheReusesUnchangedContentAndInvalidatesEditsReloadsAndColors)
{
    auto t = SmallTerrain();
    TerrainGeometry cache;
    EXPECT_TRUE(cache.Update(t));
    const auto* vertices = Batch(cache.Batches(), TerrainBatch::BlackSurface).vertices.data();
    EXPECT_FALSE(cache.Update(TerrainRenderData(t)));
    EXPECT_EQ(vertices, Batch(cache.Batches(), TerrainBatch::BlackSurface).vertices.data());
    t.heightMap[2][2] = 3; EXPECT_TRUE(cache.Update(t)); EXPECT_FALSE(cache.Update(t));
    t.floatMap[1][1] = 4; EXPECT_TRUE(cache.Update(t));
    t.colors.defaultColor.x = .9f; EXPECT_TRUE(cache.Update(t));
    t.colors.blockColor.z = .9f; EXPECT_TRUE(cache.Update(t));
    t.levelNumber = 50; EXPECT_TRUE(cache.Update(t));
    t.sizeX = 6; EXPECT_TRUE(cache.Update(t));
    EXPECT_TRUE(cache.Update(SmallTerrain())); // Same level id, different content.
    cache.Clear(); EXPECT_TRUE(cache.Update(SmallTerrain()));
}

TEST(TerrainGeometryTest, CacheIgnoresUnusedFieldsAndCells)
{
    auto t = SmallTerrain(); TerrainGeometry cache; cache.Update(t);
    t.heightMap[127][127] = 99; t.floatMap[127][127] = 5;
    t.drawFloor = false; t.drawWalls = true; t.drawTop = true;
    t.colorNumber = 12; t.colors.specialColor1.x = .42f;
    EXPECT_FALSE(cache.Update(t)); // Active compatibility path ignores these too.
}

TEST(TerrainGeometryTest, RejectsInvalidDimensionsBeforeReadingMaps)
{
    auto t = SmallTerrain();
    for (int size : {-1, 0, 1, 129}) {
        t.sizeX = size;
        EXPECT_THROW(TerrainGeometry::Build(t), std::invalid_argument);
    }
    t.sizeX = 5; t.sizeZ = 129;
    EXPECT_THROW(TerrainGeometry::Build(t), std::invalid_argument);
}

TEST(TerrainGeometryTest, LightingNormalsPreserveInheritedUpDirection)
{
    TerrainRenderData terrain;
    terrain.sizeX = terrain.sizeZ = 4;
    terrain.floatMap[2][2] = 3;
    terrain.heightMap[2][2] = 2;
    const auto batches = TerrainGeometry::Build(terrain);
    for (const auto& batch : batches) {
        EXPECT_TRUE(batch.hasNormals);
        for (const auto& vertex : batch.vertices) {
            EXPECT_FLOAT_EQ(vertex.normalX, 0);
            EXPECT_FLOAT_EQ(vertex.normalY, 1);
            EXPECT_FLOAT_EQ(vertex.normalZ, 0);
        }
    }
}

TEST(TerrainGeometryTest, WaterRibbonsSurroundNegativeCellWithLegacyUVsAndHeight)
{
    auto terrain = SmallTerrain();
    EXPECT_TRUE(Batch(TerrainGeometry::Build(terrain), TerrainBatch::Water).vertices.empty());
    terrain.heightMap[2][2] = -3;
    const auto batches = TerrainGeometry::Build(terrain);
    const auto& water = Batch(batches, TerrainBatch::Water);
    ASSERT_EQ(water.vertices.size(), 16u);
    // Each ribbon starts at the preceding (zero) height, regardless of pit depth.
    const float expected[4][4] = {{2,2,3,2}, {2,3,3,3}, {2,2,2,3}, {3,2,3,3}};
    for (int face = 0; face < 4; ++face) {
        const auto* v = &water.vertices[face*4];
        EXPECT_EQ(v[0].x, expected[face][0]); EXPECT_EQ(v[0].z, expected[face][1]);
        EXPECT_EQ(v[1].x, expected[face][2]); EXPECT_EQ(v[1].z, expected[face][3]);
        EXPECT_EQ(v[0].y, 2); EXPECT_EQ(v[1].y, 2);
        EXPECT_EQ(v[2].y, 0); EXPECT_EQ(v[3].y, 0);
        EXPECT_EQ(v[0].u, 0); EXPECT_EQ(v[0].v, 1);
        EXPECT_EQ(v[1].u, 1); EXPECT_EQ(v[1].v, 1);
        EXPECT_EQ(v[2].u, 1); EXPECT_EQ(v[2].v, 0);
        EXPECT_EQ(v[3].u, 0); EXPECT_EQ(v[3].v, 0);
        EXPECT_FLOAT_EQ(v[0].red, .2f);
        EXPECT_FLOAT_EQ(v[0].normalY, 1);
    }
}

TEST(TerrainGeometryTest, WaterRetainsPreviousHeightAcrossRowsAndRefreshesCachedContent)
{
    auto terrain = SmallTerrain();
    terrain.heightMap[0][4] = 3;
    terrain.heightMap[1][0] = -2;
    TerrainGeometry cache;
    ASSERT_TRUE(cache.Update(terrain));
    const auto& water = Batch(cache.Batches(), TerrainBatch::Water);
    ASSERT_GE(water.vertices.size(), 4u);
    EXPECT_EQ(water.vertices[0].x, 1);
    EXPECT_EQ(water.vertices[0].z, 0);
    EXPECT_EQ(water.vertices[0].y, 5); // Previous row's final height + 2.
    EXPECT_EQ(water.vertices[2].y, 3);
    const auto* vertices = water.vertices.data();
    EXPECT_FALSE(cache.Update(terrain));
    EXPECT_EQ(Batch(cache.Batches(), TerrainBatch::Water).vertices.data(), vertices);
    terrain.heightMap[0][4] = 1;
    ASSERT_TRUE(cache.Update(terrain));
    EXPECT_EQ(Batch(cache.Batches(), TerrainBatch::Water).vertices[0].y, 3);
}
