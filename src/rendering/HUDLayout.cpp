#include "HUDLayout.h"
#include <algorithm>
#include <cmath>

namespace HUDLayout {
namespace {
float Fraction(float value, float maximum) {
    if (!std::isfinite(value) || !std::isfinite(maximum) || maximum <= 0) return 0;
    return std::max(0.0f, std::min(1.0f, value / maximum));
}
}

const HUDRenderData* FindPlayer(const std::vector<HUDRenderData>& players, int playerId) {
    for (const auto& player : players) if (player.playerId == playerId) return &player;
    return nullptr;
}

int SpecialIndicatorCount(const HUDRenderData& data) {
    const float count = (data.special / 100.0f) / (data.fireCost / 500.0f);
    if (data.fireCost <= 0 || !std::isfinite(count) || count <= 0) return 0;
    // Bound malformed DTOs without changing the current game's cost range.
    return static_cast<int>(std::min(count, 1024.0f));
}

std::vector<Draw> Build(const HUDRenderData& data) {
    std::vector<Draw> draws;
    const auto bar = [&](float y, float value, float maximum, const Vector3& color,
                         const Vector3& border, Icon icon, const Vector3& tint) {
        draws.push_back({Shape::Quad, -.95f, y, .40f, .04f, Vector3(.2f,.2f,.2f)});
        draws.push_back({Shape::Quad, -.95f, y, .40f * Fraction(value, maximum), .04f, color});
        // Start at the upper-left corner, as in the compatibility line loop.
        draws.push_back({Shape::Outline, -.95f, y + .04f, .40f, -.04f, border});
        draws.push_back({Shape::Quad, -.95f - .04f, y, .03f, .04f, tint, 1, icon, Blend::Additive});
    };
    if (data.showHealthBar) bar(.90f, data.health, data.maxHealth, data.healthColor,
        Vector3(1,.8f,.8f), Icon::Health, Vector3(1,.6f,.6f));
    if (data.showEnergyBar) {
        bar(.85f, data.energy, data.maxEnergy, data.energyColor,
            Vector3(.5f,1,1), Icon::Energy, Vector3(.6f,.6f,1));
        if (data.maxEnergy > 0) {
            draws.push_back({Shape::Line, -.95f + .29f * (data.fireCost/data.maxEnergy), .86f,
                0, .03f, data.canFire ? Vector3(1,.8f,.2f) : Vector3(.8f,.4f,.4f)});
            if (data.jumpCost != data.fireCost) draws.push_back({Shape::Line,
                -.95f + .29f * (data.jumpCost/data.maxEnergy), .86f, 0, .03f, Vector3(.2f,1,.8f)});
        }
    }
    if (data.showComboMeter) draws.push_back({Shape::Quad, .50f, -.37f, .01f, data.combo/100, data.comboColor});
    if (data.showSpecialMeter) {
        draws.push_back({Shape::Quad, .52f, -.37f, .01f, data.special/100, data.specialColor,
            data.hasSpecialAvailable ? 1.0f : .5f, Icon::None,
            data.hasSpecialAvailable ? Blend::Opaque : Blend::Alpha});
        const float step = data.fireCost / 500.0f;
        for (int i = 1; i <= SpecialIndicatorCount(data); ++i)
            draws.push_back({Shape::Line, .52f, -.37f + step*i, .01f, 0, Vector3(1,1,1)});
        draws.push_back({Shape::Line, .52f, -.37f, .01f, 0, Vector3(1,1,1)});
    }
    if (data.showDebugInfo) {
        const float value = 2 * data.deltaTime;
        draws.push_back({Shape::Quad, .50f, -.30f, .01f, value, Vector3(1,value,1)});
    }
    return draws;
}
}
