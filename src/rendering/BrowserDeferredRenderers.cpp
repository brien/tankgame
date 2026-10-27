#ifdef __EMSCRIPTEN__

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
