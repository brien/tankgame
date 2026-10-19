#ifdef __EMSCRIPTEN__

#include "HUDRenderer.h"
#include "TankRenderer.h"

#include "../Logger.h"

namespace
{
void LogDeferred(const char* subsystem)
{
    Logger::Get().Write(
        "Browser renderer unavailable: %s is intentionally deferred; visual submission will be skipped.\n",
        subsystem);
}
}

HUDRenderer::HUDRenderer() : texturesLoaded(false) {}

bool HUDRenderer::Initialize()
{
    LogDeferred("HUD text (targeting/meters/icons use the shared renderer)");
    return BaseRenderer::Initialize();
}

void HUDRenderer::Cleanup() { BaseRenderer::Cleanup(); }
void HUDRenderer::RenderPlayerHUD(const HUDRenderData&) {}
void HUDRenderer::RenderAllPlayerHUDs(const std::vector<HUDRenderData>&) {}
void HUDRenderer::RenderCompleteUI(const UIRenderData&) {}
void HUDRenderer::RenderMenu(const MenuRenderData&) {}
void HUDRenderer::RenderDebugInfo(const DebugRenderData&) {}

TankRenderer::TankRenderer() : animationDrift(0.0f) {}

bool TankRenderer::Initialize()
{
    LogDeferred("compatibility unified tank renderer");
    return BaseRenderer::Initialize();
}

void TankRenderer::Cleanup() { BaseRenderer::Cleanup(); }
void TankRenderer::Render(const TankRenderData&) {}
void TankRenderer::RenderMultiple(const std::vector<TankRenderData>&) {}
void TankRenderer::SetupRenderState() {}
void TankRenderer::CleanupRenderState() {}

#endif
