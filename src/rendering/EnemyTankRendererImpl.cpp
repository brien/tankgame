#include "EnemyTankRendererImpl.h"

#include "../App.h"
#include "RenderContext.h"
#include "RendererMode.h"

EnemyTankRendererImpl::EnemyTankRendererImpl(ResourceManager* resources)
    : resources(resources) {
}

namespace {
BasicMaterial EnemyMaterial(const Color& color, float health, float maxHealth) {
    BasicMaterial material;
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
    const Matrix4 aim = body * Matrix4::Rotation(tank.turretRotation.x, 1, 0, 0) *
        Matrix4::Rotation(-tank.turretRotation.y, 0, 1, 0) *
        Matrix4::Rotation(tank.turretRotation.z, 0, 0, 1);
    const BasicMaterial secondary = EnemyMaterial(tank.secondaryColor, tank.health, tank.maxHealth);
    const BasicMaterial primary = EnemyMaterial(tank.primaryColor, tank.health, tank.maxHealth);
    // Compatibility restores each component scale before applying relative aim
    // and the 0.1 local-X cannon offset. Never inherit the body's 0.06 scale.
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
    
    if (RendererMode::IsModern()) {
        RenderEnemyTankGeometry(data);
        return;
    }
#ifndef __EMSCRIPTEN__
    // Save matrix state
    glPushMatrix();
    
    // Render enemy tank using hardcoded geometry for performance
    RenderEnemyTankGeometry(data);
    
    // Restore matrix state
    glPopMatrix();
#endif
}

void EnemyTankRendererImpl::SetupRenderState() {
    if (RendererMode::IsModern()) {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CW);
        return;
    }
#ifndef __EMSCRIPTEN__
    BaseRenderer::SetupRenderState();
    // Enemy tank render state (optimized for performance)
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glFrontFace(GL_CW);
#endif
}

void EnemyTankRendererImpl::CleanupRenderState() {
    if (RendererMode::IsModern()) return;
#ifndef __EMSCRIPTEN__
    glPopAttrib();
    BaseRenderer::CleanupRenderState();
#endif
}

void EnemyTankRendererImpl::RenderEnemyTankGeometry(const TankRenderData& tank) {
    if (RendererMode::IsModern()) {
        ResourceManager& catalogue = resources ? *resources : App::GetSingleton().graphicsTask->Resources();
        for (const EnemyTankDraw& draw : BuildDraws(tank)) {
            RenderContext::Current().Draw(catalogue.GetGeometry(draw.geometry), draw.model, draw.material);
        }
        return;
    }
#ifndef __EMSCRIPTEN__
    // Replicate TankRenderer::Draw() logic exactly for compatibility
    
    // Setup body transform
    SetupBodyTransform(tank);
    
    // Draw body with secondary colors and health-based modification
    DrawBody(tank.secondaryColor.r, tank.secondaryColor.g, tank.secondaryColor.b, tank.health, tank.maxHealth);
    
    // Setup barrel transform
    SetupBarrelTransform(tank);
    
    // Draw barrel with primary colors and health-based modification
    DrawBarrel(tank.primaryColor.r, tank.primaryColor.g, tank.primaryColor.b, tank.health, tank.maxHealth);
    
    // Setup turret transform
    SetupTurretTransform();
    
    // Draw turret (uses current color from barrel)
    DrawTurret();
#endif
}

#ifndef __EMSCRIPTEN__
void EnemyTankRendererImpl::SetupBodyTransform(const TankRenderData& tank) {
    glTranslatef(tank.position.x, tank.position.y, tank.position.z);
    glRotatef(tank.bodyRotation.x, 1, 0, 0);
    glRotatef(-tank.bodyRotation.y, 0, 1, 0);
    glRotatef(tank.bodyRotation.z, 0, 0, 1);
}

void EnemyTankRendererImpl::SetupBarrelTransform(const TankRenderData& tank) {
    glRotatef(tank.turretRotation.x, 1, 0, 0);
    glRotatef(-tank.turretRotation.y, 0, 1, 0);
    glRotatef(tank.turretRotation.z, 0, 0, 1);
}

void EnemyTankRendererImpl::SetupTurretTransform() {
    glTranslatef(0.1f, 0, 0);
}

void EnemyTankRendererImpl::DrawBody(float r, float g, float b, float energy, float maxEnergy) {
    // Exact copy from TankRenderer::DrawBody for consistency
    if (maxEnergy > 0.0f && energy > 0.0f) {
        float healthFactor = maxEnergy / energy;
        glColor3f((4 * r + healthFactor) / 2, (4 * g + healthFactor) / 2, (4 * b + healthFactor) / 2);
    } else {
        glColor3f(r, g, b);
    }
    
    glScalef(.06f, .06f, .06f);
    glBegin(GL_TRIANGLES);
    
    // Tank body geometry (exact copy from TankRenderer.cpp)
    glNormal3f(0.894427f, -0.447214f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(4, 0, -4);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(4, 0, 4);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(5, 2, 3);
    
    glNormal3f(0.894427f, -0.447214f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(4, 0, -4);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(5, 2, 3);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(5, 2, -3);
    
    glNormal3f(-0.894427f, -0.447214f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(-4, 0, -4);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(-5, 2, -5);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-5, 2, 5);
    
    glNormal3f(-0.894427f, -0.447214f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(-4, 0, -4);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-5, 2, 5);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-4, 0, 4);
    
    glNormal3f(0.182574f, 0.365148f, -0.912871f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(4, 0, -4);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(5, 2, -3);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(-5, 2, -5);
    
    glNormal3f(0.000000f, -0.447214f, -0.894427f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(4, 0, -4);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(-5, 2, -5);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(-4, 0, -4);
    
    glNormal3f(0.000000f, -0.447214f, 0.894427f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(4, 0, 4);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-4, 0, 4);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-5, 2, 5);
    
    glNormal3f(0.182574f, 0.365148f, 0.912871f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(4, 0, 4);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-5, 2, 5);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(5, 2, 3);
    
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(5, 2, -3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(5, 2, 3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-5, 2, 5);
    
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(5, 2, -3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-5, 2, 5);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(-5, 2, -5);
    
    glNormal3f(0.000000f, -1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(4, 0, -4);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(-4, 0, -4);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-4, 0, 4);
    
    glNormal3f(0.000000f, -1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(4, 0, -4);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-4, 0, 4);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(4, 0, 4);
    
    glEnd();
    glScalef(1.0f/0.06f, 1.0f/0.06f, 1.0f/0.06f); // Restore scale
}

void EnemyTankRendererImpl::DrawBarrel(float r, float g, float b, float energy, float maxEnergy) {
    // Exact copy from TankRenderer::DrawBarrel for consistency
    if (maxEnergy > 0.0f && energy > 0.0f) {
        float healthFactor = maxEnergy / energy;
        glColor3f((4 * r + healthFactor) / 2, (4 * g + healthFactor) / 2, (4 * b + healthFactor) / 2);
    } else {
        glColor3f(r, g, b);
    }
    
    glScalef(.1f, .1f, .1f);
    glBegin(GL_TRIANGLES);
    
    // Tank barrel geometry (exact copy from TankRenderer.cpp)
    glNormal3f(0.163846f, -0.081923f, 0.983078f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, 2);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, 3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, 3);
    
    glNormal3f(0.180156f, -0.041001f, 0.982783f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, 2);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, 3);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, 2);
    
    glNormal3f(0.196116f, 0.000000f, -0.980581f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, -2);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, -2);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, -3);
    
    glNormal3f(0.180156f, -0.041001f, -0.982783f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, -2);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, -3);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, -3);
    
    glNormal3f(1.000000f, 0.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, 2);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, 2);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, -2);
    
    glNormal3f(1.000000f, 0.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, 2);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, -2);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, -2);
    
    glNormal3f(-0.894427f, 0.447214f, 0.000000f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, 3);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, -3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, -3);
    
    glNormal3f(-0.894427f, 0.447214f, 0.000000f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, 3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, -3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, 3);
    
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, 2);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, 3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, -3);
    
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, 2);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, -3);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, -2);
    
    glNormal3f(0.000000f, -1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, 2);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, -2);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, -3);
    
    glNormal3f(0.000000f, -1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, 2);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, -3);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, 3);
    
    glEnd();
    glScalef(1.0f/0.1f, 1.0f/0.1f, 1.0f/0.1f); // Restore scale
}

void EnemyTankRendererImpl::DrawTurret() {
    // Exact copy from TankRenderer::DrawTurret for consistency
    glScalef(.1f, .1f, .1f);
    glBegin(GL_TRIANGLES);
    
    // Tank turret geometry (exact copy from TankRenderer.cpp)
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, -0.707107f);
    glVertex3f(-3, 4, -0.25f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3f(5, 4, -0.25f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3f(5, 4, 0.25f);
    
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3f(-3, 4, -0.25f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3f(5, 4, 0.25f);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3f(-3, 4, 0.25f);
    
    glNormal3f(0.000000f, -0.707107f, -0.707107f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3f(-3, 4, -0.25f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(-3, 3, 0);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(5, 3, 0);
    
    glNormal3f(0.000000f, -0.707107f, -0.707107f);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3f(-3, 4, -0.25f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(5, 3, 0);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3f(5, 4, -0.25f);
    
    glNormal3f(-1.000000f, 0.000000f, 0.000000f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3f(-3, 4, -0.25f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3f(-3, 4, 0.25f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(-3, 3, 0);
    
    glNormal3f(1.000000f, 0.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3f(5, 4, -0.25f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(5, 3, 0);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3f(5, 4, 0.25f);
    
    glNormal3f(0.000000f, -0.707107f, 0.707107f);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3f(-3, 4, 0.25f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3f(5, 4, 0.25f);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(5, 3, 0);
    
    glNormal3f(0.000000f, -0.707107f, 0.707107f);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3f(-3, 4, 0.25f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(5, 3, 0);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-3, 3, 0);
    
    glEnd();
    glScalef(1.0f/0.1f, 1.0f/0.1f, 1.0f/0.1f); // Restore scale
}

#endif
