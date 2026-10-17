#pragma once

#include "RenderData.h"
#include "Matrix4.h"

class ResourceManager;

// World-space diagnostic lines. The catalogue owns one immutable segment;
// endpoints and colour vary through transforms/materials, never GPU uploads.
class DebugLineRenderer {
public:
    static Matrix4 BuildModel(const DebugLineData& line);
    static std::vector<DebugLineData> BuildTankMarkers(const std::vector<TankRenderData>& tanks);
    static void Render(ResourceManager& resources, const std::vector<DebugLineData>& lines);
};
