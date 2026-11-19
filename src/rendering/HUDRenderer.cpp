#include "HUDRenderer.h"
#include "HUDLayout.h"
#include "HUDMeterRenderer.h"
#include "TargetingLineRenderer.h"
#include "TextRenderer.h"
#include "ResourceManager.h"
#include "GLFunctions.h"
#include <cmath>

HUDRenderer::HUDRenderer(ResourceManager& resources) : resources(resources) {}

bool HUDRenderer::Initialize() { return true; }
void HUDRenderer::Cleanup() {}

void HUDRenderer::RenderPlayerHUD(const HUDRenderData& data) {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    TargetingLineRenderer::Render(resources, data);
    HUDMeterRenderer::Render(resources, data);
    TextRenderer::Render(resources, TextRenderer::PlayerLines(data, viewport[2]));
    TextRenderer::Render(resources, TextRenderer::StatusLines(data, viewport[2], viewport[3]));
}

void HUDRenderer::RenderDebugInfo(const DebugRenderData& data) {
    TextRenderer::Render(resources, TextRenderer::DebugLines(data));
}
