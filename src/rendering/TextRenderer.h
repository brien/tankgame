#pragma once
#include "HUDData.h"
#include <string>
#include <vector>
class ResourceManager;
struct UITextLine {
    std::string text;
    float x, y; // drawable pixels from the viewport's upper left
    Vector3 color;
};
namespace TextRenderer {
std::vector<UITextLine> PlayerLines(const HUDRenderData& data, int width);
std::vector<UITextLine> DebugLines(const DebugRenderData& data);
void Render(ResourceManager& resources, const std::vector<UITextLine>& lines);
}
