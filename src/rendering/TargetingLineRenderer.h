#pragma once

#include "HUDData.h"
#include "Matrix4.h"
#include "BasicMaterial.h"

class ResourceManager;

namespace TargetingLineRenderer {
Matrix4 BuildModel(const HUDRenderData& data);
// Returns false for hidden, invalid, fully clipped or screen-degenerate rays.
bool BuildMaterial(const HUDRenderData& data, const Matrix4& mvp,
                   const int viewport[4], BasicMaterial& material);
void Render(ResourceManager& resources, const HUDRenderData& data);
}
