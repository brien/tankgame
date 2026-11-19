#pragma once

#include "ITankRenderer.h"
#include "BaseRenderer.h"
#include "RenderData.h"
#include "ResourceManager.h"
#include "Matrix4.h"
#include <array>

// Shared shader-backed enemy submission.
struct EnemyTankDraw {
    GeometryResource geometry;
    Matrix4 model;
    BasicMaterial material;
};

class EnemyTankRendererImpl : public BaseRenderer, public ITankRenderer {
public:
    explicit EnemyTankRendererImpl(ResourceManager* resources = nullptr);

    // GL-free draw description used by submission and regression tests.
    static std::array<EnemyTankDraw, 3> BuildDraws(const TankRenderData& tank);
    virtual ~EnemyTankRendererImpl() = default;
    
    // ITankRenderer interface implementation
    bool Initialize() override;
    void Cleanup() override;
    void Render(const TankRenderData& data) override;
    void SetupRenderState() override;
    void CleanupRenderState() override;
    
    // EnemyTankRenderer-specific methods
    void RenderEnemyTankGeometry(const TankRenderData& tank);
    
private:
    ResourceManager* resources; // Non-owning; catalogue outlives the pipeline.
};
