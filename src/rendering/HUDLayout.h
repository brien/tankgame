#pragma once

#include "HUDData.h"

// GL-free descriptions of the existing player-local HUD. Coordinates are in
// normalized viewport space; every draw uses an immutable catalogue primitive.
namespace HUDLayout {
enum class Shape { Quad, Outline, Line };
enum class Icon { None, Health, Energy };
enum class Blend { Opaque, Alpha, Additive };
struct Draw {
    Shape shape;
    float x, y, width, height;
    Vector3 color;
    float alpha = 1.0f;
    Icon icon = Icon::None;
    Blend blend = Blend::Opaque;
};
std::vector<Draw> Build(const HUDRenderData& data);
int SpecialIndicatorCount(const HUDRenderData& data);
const HUDRenderData* FindPlayer(const std::vector<HUDRenderData>& players, int playerId);
}
