#include "EnemyTankRendererImpl.h"

#include "../App.h"
#include "RenderContext.h"

EnemyTankRendererImpl::EnemyTankRendererImpl(ResourceManager* resources)
    : resources(resources) {
}

namespace {
BasicMaterial EnemyMaterial(const Color& color, float health, float maxHealth) {
    BasicMaterial material;
    material.lit = true;
    // Preserve glColor3f's multiplier (including values above one); the shader
    // and framebuffer perform the final clamp. Enemy surfaces are opaque/untextured.
    const bool hasHealth = health > 0.0f && maxHealth > 0.0f;
    const float factor = hasHealth ? maxHealth / health : 0.0f;
    material.red = hasHealth ? (4 * color.r + factor) / 2 : color.r;
    material.green = hasHealth ? (4 * color.g + factor) / 2 : color.g;
    material.blue = hasHealth ? (4 * color.b + factor) / 2 : color.b;
    return material;
}
}

std::array<EnemyTankDraw, 3> EnemyTankRendererImpl::BuildDraws(const TankRenderData& tank) {
    const Matrix4 body = Matrix4::Translation(tank.position.x, tank.position.y, tank.position.z) *
        Matrix4::Rotation(tank.bodyRotation.x, 1, 0, 0) *
        Matrix4::Rotation(-tank.bodyRotation.y, 0, 1, 0) *
        Matrix4::Rotation(tank.bodyRotation.z, 0, 0, 1);
    const Matrix4 aim = body * Matrix4::Scale(.6f, .6f, .6f) * Matrix4::Rotation(tank.turretRotation.x, 1, 0, 0) *
        Matrix4::Rotation(-tank.turretRotation.y, 0, 1, 0) *
        Matrix4::Rotation(tank.turretRotation.z, 0, 0, 1);
    const BasicMaterial secondary = EnemyMaterial(tank.secondaryColor, tank.health, tank.maxHealth);
    const BasicMaterial primary = EnemyMaterial(tank.primaryColor, tank.health, tank.maxHealth);
    // The native TankRenderer scales the body by 0.06, then by 10, leaving
    // a 0.6 scale for the upper assembly and its local-X cannon offset.
    return {{{GeometryResource::EnemyBody, body * Matrix4::Scale(.06f, .06f, .06f), secondary},
             {GeometryResource::EnemyBarrel, aim * Matrix4::Scale(.1f, .1f, .1f), primary},
             {GeometryResource::EnemyTurret, aim * Matrix4::Translation(.1f, 0, 0) *
                 Matrix4::Scale(.1f, .1f, .1f), primary}}};
}

bool EnemyTankRendererImpl::Initialize() {
    if (!BaseRenderer::Initialize()) {
        return false;
    }
    return true;
}

void EnemyTankRendererImpl::Cleanup() {
    BaseRenderer::Cleanup();
}

void EnemyTankRendererImpl::Render(const TankRenderData& data) {
    if (!data.alive || data.isPlayer) {
        return;
    }
    
    RenderEnemyTankGeometry(data);
}

void EnemyTankRendererImpl::SetupRenderState() {
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
}

void EnemyTankRendererImpl::CleanupRenderState() {
}

void EnemyTankRendererImpl::RenderEnemyTankGeometry(const TankRenderData& tank) {
    ResourceManager& catalogue = resources ? *resources : App::GetSingleton().graphicsTask->Resources();
    for (const EnemyTankDraw& draw : BuildDraws(tank)) {
        RenderContext::Current().Draw(catalogue.GetGeometry(draw.geometry), draw.model, draw.material);
    }
}
