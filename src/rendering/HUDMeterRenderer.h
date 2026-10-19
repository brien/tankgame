#pragma once

class ResourceManager;
struct HUDRenderData;

// Shared programmable HUD slice. Targeting stipple and font rendering are
// deliberately separate from the viewport-local meters and icons.
class HUDMeterRenderer {
public:
    static void Render(ResourceManager& resources, const HUDRenderData& data);
};
