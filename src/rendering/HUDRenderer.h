#pragma once

#include "BaseRenderer.h"
#include "HUDData.h"

class ResourceManager;

// Player-local targeting/meters/text and global diagnostics. Resources are
// borrowed from the pipeline catalogue; menus belong to MenuRenderer.
class HUDRenderer : public BaseRenderer {
public:
    explicit HUDRenderer(ResourceManager& resources);
    bool Initialize() override;
    void Cleanup() override;
    void RenderPlayerHUD(const HUDRenderData& data);
    void RenderDebugInfo(const DebugRenderData& data);

private:
    ResourceManager& resources;
#ifndef __EMSCRIPTEN__
    void RenderTargetingLine(const HUDRenderData& data);
#endif
};
