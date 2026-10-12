#include "TerrainGeometry.h"
#include <algorithm>
#include <stdexcept>

namespace
{
void Validate(const TerrainRenderData& t)
{
    if (t.sizeX < 2 || t.sizeZ < 2 ||
        t.sizeX > TerrainRenderData::MAX_SIZE_X || t.sizeZ > TerrainRenderData::MAX_SIZE_Z)
        throw std::invalid_argument("terrain dimensions outside render map bounds");
}

GeometryVertex Vertex(float x, float y, float z, float u, float v, const Vector3& c)
{
    return {x, y, z, u, v, 0, 1, 0, c.x, c.y, c.z};
}

void Quad(Geometry& geometry, GeometryVertex a, GeometryVertex b,
          GeometryVertex c, GeometryVertex d)
{
    geometry.vertices.insert(geometry.vertices.end(), {a, b, c, d});
}

TerrainBatch SurfaceBatch(int level, int height)
{
    if (height != 0) {
        if (level == 0 || level == 50 || level == 56 || level == 57 || level == 58)
            return TerrainBatch::CheckerSurface;
        if (level == 48 || level == 69 || level == 70)
            return TerrainBatch::WhiteSurface;
    }
    return TerrainBatch::BlackSurface;
}

void Wall(Geometry& g, const TerrainRenderData& t, int x, int z, int last, int height, int direction)
{
    // The compatibility loop emits only on upward transitions (including a
    // degenerate quad for equal heights). Keep its coordinates and UV direction.
    if (last > height) return;
    const auto& c = last < 0 ? t.colors.blockColor : t.colors.defaultColor;
    const float span = height - last;
    switch (direction) {
    case 0:
        Quad(g, Vertex(x,last,z,0,0,c), Vertex(x+1,last,z,1,0,c),
             Vertex(x+1,height,z,1,span,c), Vertex(x,height,z,0,span,c)); break;
    case 1:
        Quad(g, Vertex(x,height,z+1,0,0,c), Vertex(x+1,height,z+1,1,0,c),
             Vertex(x+1,last,z+1,1,span,c), Vertex(x,last,z+1,0,span,c)); break;
    case 2:
        Quad(g, Vertex(x,height,z,0,0,c), Vertex(x,height,z+1,1,0,c),
             Vertex(x,last,z+1,1,span,c), Vertex(x,last,z,0,span,c)); break;
    case 3:
        Quad(g, Vertex(x+1,last,z,0,0,c), Vertex(x+1,last,z+1,1,0,c),
             Vertex(x+1,height,z+1,1,span,c), Vertex(x+1,height,z,0,span,c)); break;
    }
}

bool SameColor(const Vector3& a, const Vector3& b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}
}

TerrainBatches TerrainGeometry::Build(const TerrainRenderData& t)
{
    Validate(t);
    TerrainBatches result;
    for (auto& batch : result) {
        batch.hasTextureCoordinates = true;
        batch.hasColors = true;
        // Lighting is deferred. Do not invent normals for the legacy walls,
        // which inherit the surface normal in the compatibility renderer.
    }
    auto& floating = result[static_cast<std::size_t>(TerrainBatch::Floating)];
    const Geometry cube = SimpleGeometry::CreateCube();
    for (int x = 0; x < t.sizeX; ++x) {
        for (int z = 0; z < t.sizeZ; ++z) {
            if (!t.floatMap[x][z]) continue;
            for (auto v : cube.vertices) {
                v.x += x + .5f; v.y += t.floatMap[x][z] - .5f; v.z += z + .5f;
                v.red = t.colors.blockColor.x;
                v.green = t.colors.blockColor.y;
                v.blue = t.colors.blockColor.z;
                floating.vertices.push_back(v);
            }
        }
    }

    const int sx = t.sizeX - 1, sz = t.sizeZ - 1;
    // Deliberately retain the original strip endpoints, last-strip extension,
    // negative-height omission, and >=25 cutoff. These are rendering semantics,
    // not a new interpretation of collision cells or drawFloor/drawWalls flags.
    for (int x = 0; x < sx; ++x) {
        int height = t.heightMap[x][0], strips = 0;
        for (int z = 0; z < sz; ++z) {
            ++strips;
            const bool last = z == sz - 1;
            if (height != t.heightMap[x][z] || last) {
                if (height < 25 && (last || height >= 0)) {
                    auto& g = result[static_cast<std::size_t>(SurfaceBatch(t.levelNumber, height))];
                    const auto& c = height == 0 ? t.colors.blockColor : t.colors.defaultColor;
                    const int end = last ? z + 1 : z;
                    const int repeat = last ? strips + 1 : strips;
                    Quad(g, Vertex(x,height,z-strips,repeat,1,c), Vertex(x+1,height,z-strips,repeat,0,c),
                         Vertex(x+1,height,end,0,0,c), Vertex(x,height,end,0,1,c));
                }
                strips = 0;
            }
            height = t.heightMap[x][z];
        }
    }

    auto& walls = result[static_cast<std::size_t>(TerrainBatch::Walls)];
    for (int x = 0; x <= sx; ++x) {
        int last = 0;
        for (int z = 0; z <= sz; ++z) {
            const int h = t.heightMap[x][z];
            if (h != last && z != sz && x != 0) Wall(walls,t,x,z,last,h,0);
            last = h;
        }
    }
    for (int x = sx; x > 0; --x) {
        int last = 0;
        for (int z = sz-1; z > 0; --z) {
            const int h = t.heightMap[x][z];
            if (h != last && x != sx) Wall(walls,t,x,z,last,h,1);
            last = h;
        }
    }
    for (int z = 0; z < sz; ++z) {
        int last = 0;
        for (int x = 0; x <= sx; ++x) {
            const int h = t.heightMap[x][z];
            if (h != last && x != sx && z != 0) Wall(walls,t,x,z,last,h,2);
            last = h;
        }
    }
    for (int z = sz; z > 0; --z) {
        int last = 0;
        for (int x = sx; x >= 0; --x) {
            const int h = t.heightMap[x][z];
            if (h != last && x != 0) Wall(walls,t,x,z,last,h,3);
            last = h;
        }
    }

    auto& boundary = result[static_cast<std::size_t>(TerrainBatch::Boundary)];
    const auto& c = t.colors.defaultColor;
    const auto& b = t.colors.blockColor;
    const int top = t.levelNumber == 48 ? 0 : 30;
    const int bottom = top - 30;
    // The legacy boundary uses sz for two X coordinates, even on rectangular
    // maps. Preserve that behavior rather than silently fixing level geometry.
    if (t.levelNumber != 48)
        Quad(boundary, Vertex(1,26,1,sx-1,sz,c), Vertex(1,26,sz,0,sz,c),
             Vertex(sz,26,sz,0,0,c), Vertex(sz,26,1,sx-1,0,c));
    Quad(boundary, Vertex(1,top,1,0,30,c), Vertex(sx,top,1,sx-1,30,c),
         Vertex(sx,bottom,1,sx-1,0,b), Vertex(1,bottom,1,0,0,b));
    Quad(boundary, Vertex(sx,top,sz,sx-1,30,c), Vertex(1,top,sz,0,30,c),
         Vertex(1,bottom,sz,0,0,b), Vertex(sx,bottom,sz,sx-1,0,b));
    Quad(boundary, Vertex(sz,top,1,0,30,c), Vertex(sx,top,sz,sx-1,30,c),
         Vertex(sx,bottom,sz,sx-1,0,b), Vertex(sz,bottom,1,0,0,b));
    Quad(boundary, Vertex(1,top,sz,0,30,c), Vertex(1,top,1,sx-1,30,c),
         Vertex(1,bottom,1,sx-1,0,b), Vertex(1,bottom,sz,0,0,b));
    return result;
}

bool TerrainGeometry::Matches(const TerrainRenderData& a, const TerrainRenderData& b)
{
    Validate(a); Validate(b);
    if (a.sizeX != b.sizeX || a.sizeZ != b.sizeZ || a.levelNumber != b.levelNumber ||
        !SameColor(a.colors.defaultColor, b.colors.defaultColor) ||
        !SameColor(a.colors.blockColor, b.colors.blockColor)) return false;
    for (int x = 0; x < a.sizeX; ++x) {
        if (!std::equal(a.heightMap[x], a.heightMap[x]+a.sizeZ, b.heightMap[x]) ||
            !std::equal(a.floatMap[x], a.floatMap[x]+a.sizeZ, b.floatMap[x])) return false;
    }
    return true;
}

bool TerrainGeometry::Update(const TerrainRenderData& terrain)
{
    if (snapshot && Matches(*snapshot, terrain)) return false;
    auto next = Build(terrain);
    auto nextSnapshot = std::unique_ptr<TerrainRenderData>(new TerrainRenderData(terrain));
    batches = std::move(next);
    snapshot = std::move(nextSnapshot);
    return true;
}

void TerrainGeometry::Clear()
{
    snapshot.reset();
    batches = TerrainBatches{};
}
