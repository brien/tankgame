#include "RenderingPipeline.h"
#include "../App.h"

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#elif __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <algorithm>
#include <chrono>

RenderingPipeline::RenderingPipeline(ViewportManager &viewport, CameraManager &camera,
                                     ResourceManager &resources)
    : viewportManager(viewport), cameraManager(camera), resourceManager(resources), renderStats{0, 0, 0, 0, 0.0f}
{
}

bool RenderingPipeline::Initialize()
{
    // Initialize all specialized renderers
    bool success = true;

    success &= terrainRenderer.Initialize();
    success &= bulletRenderer.Initialize();
    success &= effectRenderer.Initialize();
    success &= itemRenderer.Initialize();

    // Create tank renderer using factory
    tankRenderer = TankRendererFactory::CreateUnifiedRenderer();
    if (tankRenderer)
    {
        success &= tankRenderer->Initialize();
    }
    else
    {
        success = false;
    }

    return success;
}

void RenderingPipeline::Cleanup()
{
    // Cleanup all specialized renderers
    if (tankRenderer)
    {
        tankRenderer->Cleanup();
        tankRenderer.reset();
    }

    itemRenderer.Cleanup();
    effectRenderer.Cleanup();
    bulletRenderer.Cleanup();
    terrainRenderer.Cleanup();
}

void RenderingPipeline::SetupRenderState()
{
    // Setup global rendering state for the pipeline
    PushRenderState();

    // Enable depth testing for 3D rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    // Enable back-face culling for performance
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Setup alpha blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable smooth shading
    glShadeModel(GL_SMOOTH);

    // Set clear color (dark blue/black for space-like background)
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
}

void RenderingPipeline::CleanupRenderState()
{
    // Restore previous OpenGL state
    PopRenderState();
}

void RenderingPipeline::RenderScene(const SceneData &scene, int playerIndex)
{
    auto startTime = std::chrono::high_resolution_clock::now();

    // Setup scene for specific player
    SetupSceneForPlayer(scene, playerIndex);

    // Clear buffers
    ClearBuffers();

    // Render in proper order for correct depth and transparency
    RenderSkybox(scene);
    RenderTerrain(scene.terrain);
    RenderGameObjects(scene);
    RenderTransparentObjects(scene);
    RenderUIElements(scene, playerIndex);

    // Update rendering statistics
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    renderStats.renderTime = duration.count() / 1000.0f; // Convert to milliseconds

    UpdateRenderStats(scene);
}

void RenderingPipeline::RenderAllPlayerViews(const SceneData &scene)
{
    // Render for each active player
    for (int i = 0; i < scene.numPlayers && i < viewportManager.GetNumViewports(); ++i)
    {
        RenderScene(scene, i);
    }
}

void RenderingPipeline::SetupSceneForPlayer(const SceneData &scene, int playerIndex)
{
    // Validate player index
    if (playerIndex < 0 || playerIndex >= scene.numPlayers)
    {
        return;
    }

    // Setup viewport for this player
    viewportManager.SetActiveViewport(playerIndex);

    // Setup camera for this player
    if (playerIndex < static_cast<int>(scene.cameras.size()))
    {
        const CameraData &camData = scene.cameras[playerIndex];

        // Set up camera matrices
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        // Setup perspective projection
        const Viewport &viewport = viewportManager.GetViewport(playerIndex);
        float aspect = viewport.GetAspectRatio();
        gluPerspective(45.0, aspect, 0.1, 1000.0);

        // Setup view matrix
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(
            camData.position.x, camData.position.y, camData.position.z,
            camData.focus.x, camData.focus.y, camData.focus.z,
            0.0f, 1.0f, 0.0f // Standard up vector
        );
    }

    // Setup lighting for the scene
    SetupLighting(scene);
}

void RenderingPipeline::ClearBuffers()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderingPipeline::SetupLighting(const SceneData &scene)
{
    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Setup main directional light (sun)
    GLfloat lightPos[] = {0.5f, 1.0f, 0.5f, 0.0f}; // Directional light
    GLfloat lightAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    // Basic lighting adjustment based on level - could be enhanced with special lighting data
    if (scene.terrain.levelNumber >= 48)
    {
        // Brighter lighting for special levels
        GLfloat brightAmbient[] = {0.4f, 0.4f, 0.5f, 1.0f};
        glLightfv(GL_LIGHT0, GL_AMBIENT, brightAmbient);
    }
    GLfloat lavaAmbient[] = {0.3f, 0.2f, 0.1f, 1.0f};
    GLfloat lavaDiffuse[] = {1.0f, 0.6f, 0.3f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, lavaAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lavaDiffuse);
}

void RenderingPipeline::RenderSkybox(const SceneData &scene)
{
    if (scene.drawSky)
    {
        // Disable depth testing for skybox
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        // Delegate to GraphicsTask's DrawSky method for now
        // TODO: Move sky rendering to a dedicated SkyRenderer
        App::GetSingleton().graphicsTask->DrawSky();

        // Re-enable depth testing
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
    }
}

void RenderingPipeline::RenderTerrain(const TerrainRenderData &terrain)
{
    // Delegate to terrain renderer - let it handle its own state management
    terrainRenderer.RenderTerrain(terrain);
}

void RenderingPipeline::RenderGameObjects(const SceneData &scene)
{
    // Render opaque objects in front-to-back order for early Z rejection
    RenderTanks(scene.tanks);
    RenderItems(scene.items);
    RenderBullets(scene.bullets);
}

void RenderingPipeline::RenderTransparentObjects(const SceneData &scene)
{
    // Render transparent objects in back-to-front order for correct blending
    RenderEffects(scene.effects);
}

void RenderingPipeline::RenderUIElements(const SceneData &scene, int playerIndex)
{
    // TODO: Implement UI rendering (HUD, targeting, etc.)
    // This would be handled by specialized UI renderers in Phase 7

    // For now, this is a placeholder
    // UI elements would include:
    // - Health bars
    // - Ammo indicators
    // - Targeting UI
    // - Minimap
    // - Score displays
}

void RenderingPipeline::RenderTanks(const std::vector<TankRenderData> &tanks)
{
    if (tanks.empty() || !tankRenderer)
    {
        return;
    }

    tankRenderer->RenderMultiple(tanks);
    renderStats.tanksRendered = static_cast<int>(tanks.size());
}

void RenderingPipeline::RenderBullets(const std::vector<BulletRenderData> &bullets)
{
    if (bullets.empty())
    {
        return;
    }

    bulletRenderer.SetupRenderState();
    bulletRenderer.RenderBullets(bullets);
    bulletRenderer.CleanupRenderState();

    renderStats.bulletsRendered = static_cast<int>(bullets.size());
}

void RenderingPipeline::RenderEffects(const std::vector<EffectRenderData> &effects)
{
    if (effects.empty())
    {
        return;
    }

    effectRenderer.SetupRenderState();
    effectRenderer.RenderEffects(effects);
    effectRenderer.CleanupRenderState();

    renderStats.effectsRendered = static_cast<int>(effects.size());
}

void RenderingPipeline::RenderItems(const std::vector<ItemRenderData> &items)
{
    if (items.empty())
    {
        return;
    }

    // Let the ItemRenderer handle its own state management internally
    itemRenderer.RenderItems(items);

    renderStats.itemsRendered = static_cast<int>(items.size());
}

void RenderingPipeline::UpdateRenderStats(const SceneData &scene)
{
    renderStats.tanksRendered = static_cast<int>(scene.tanks.size());
    renderStats.bulletsRendered = static_cast<int>(scene.bullets.size());
    renderStats.effectsRendered = static_cast<int>(scene.effects.size());
    renderStats.itemsRendered = static_cast<int>(scene.items.size());
}

void RenderingPipeline::PushRenderState()
{
    // Save current OpenGL state
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
}

void RenderingPipeline::PopRenderState()
{
    // Restore previous OpenGL state
    glPopMatrix();
    glPopAttrib();
}

bool RenderingPipeline::ShouldRenderObject(const Vector3 &position, const Vector3 &cameraPos, float maxDistance)
{
    // Simple distance-based culling
    float dx = position.x - cameraPos.x;
    float dy = position.y - cameraPos.y;
    float dz = position.z - cameraPos.z;
    float distance = dx * dx + dy * dy + dz * dz; // Squared distance for performance

    return distance <= (maxDistance * maxDistance);
}

void RenderingPipeline::OptimizeRenderOrder(std::vector<TankRenderData> &tanks, const Vector3 &cameraPos)
{
    // Sort tanks by distance from camera (front to back for opaque objects)
    std::sort(tanks.begin(), tanks.end(),
              [&cameraPos](const TankRenderData &a, const TankRenderData &b)
              {
                  float distA = (a.position.x - cameraPos.x) * (a.position.x - cameraPos.x) +
                                (a.position.y - cameraPos.y) * (a.position.y - cameraPos.y) +
                                (a.position.z - cameraPos.z) * (a.position.z - cameraPos.z);
                  float distB = (b.position.x - cameraPos.x) * (b.position.x - cameraPos.x) +
                                (b.position.y - cameraPos.y) * (b.position.y - cameraPos.y) +
                                (b.position.z - cameraPos.z) * (b.position.z - cameraPos.z);
                  return distA < distB; // Front to back
              });
}

void RenderingPipeline::PerformFrustumCulling(const SceneData &scene, int playerIndex)
{
    // TODO: Implement view frustum culling
    // This would test objects against the camera frustum and remove those outside the view
    // For now, this is a placeholder for future optimization
}
