#pragma once

#include "../Geometry.h"
#include "RenderData.h"
#include <array>
#include <memory>

// CPU-only terrain batches. Water is submitted separately after opaque objects. Surface
// strips are grouped by texture; all use vertex colours and a white material.
enum class TerrainBatch { Floating, BlackSurface, WhiteSurface, CheckerSurface, Walls, Boundary, Water, Count };
using TerrainBatches = std::array<Geometry, static_cast<std::size_t>(TerrainBatch::Count)>;

class TerrainGeometry
{
public:
    static TerrainBatches Build(const TerrainRenderData& terrain);
    static bool Matches(const TerrainRenderData& a, const TerrainRenderData& b);
    // Content comparison also catches in-place edits and same-number reloads.
    bool Update(const TerrainRenderData& terrain);
    void Clear();
    const TerrainBatches& Batches() const { return batches; }

private:
    std::unique_ptr<TerrainRenderData> snapshot;
    TerrainBatches batches;
};
