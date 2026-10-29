//
//  GraphicsTask.cpp
//  tankgame
//
//

#ifdef _WIN32
// If building in windows:
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#elif __APPLE__
// If building on macOS:
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
// If building on Linux:
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "GraphicsTask.h"
#include "LevelHandler.h"
#include "rendering/RenderData.h"
#include "rendering/ResourceManager.h"
#include "rendering/SceneDataBuilder.h"
#include "rendering/RenderingPipeline.h"
#include "rendering/RenderContext.h"
#include "rendering/RendererMode.h"
#include <stdexcept>

#include "App.h"
#include "TankHandler.h"
#include "LevelHandler.h"
#include "VideoTask.h"

bool GraphicsTask::Start()
{
    Logger::Get().Write("GraphicsTask::Starting\n");

    // Initialize viewport manager
    viewportManager.SetupSinglePlayer(VideoTask::scrWidth, VideoTask::scrHeight);

    // Essential OpenGL setup (required for proper rendering)
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);

    // Setup viewport and projection (needed before pipeline initialization)
    glViewport(0, 0, VideoTask::scrWidth, VideoTask::scrHeight);
    if (RendererMode::IsModern()) {
    const float ratio = viewportManager.GetViewport(0).GetAspectRatio();
    RenderContext::Current().SetProjection(Matrix4::Perspective(45.0f, ratio, 0.1f, 1024.0f));
    }
#ifndef __EMSCRIPTEN__
    else {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float ratio = viewportManager.GetViewport(0).GetAspectRatio();
    gluPerspective(45.0, ratio, 0.1, 1024.0);
    }
#endif

    // Enable essential OpenGL features
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    // Desktop retains its fixed-function lighting/material setup. WebGL uses
    // explicit shader state and intentionally has no fixed-function analogue.
#ifndef __EMSCRIPTEN__
    GLfloat LightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat LightPosition[] = {0.0f, 1.0f, 0.0f, 0.0f};
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
    glPushMatrix();
    glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.1f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.05f);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glPopMatrix();

    // Enable material and texture features
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_TEXTURE_2D);
#endif

    // The GL context is current before the authoritative catalogue performs
    // any texture, buffer, shader, or compatibility display-list upload.
    resourceManager = std::make_unique<ResourceManager>();
    if (!resourceManager->Initialize())
    {
        Logger::Get().Write("ERROR: Failed to initialize renderer resources\n");
        return false;
    }

    // Initialize individual renderers (required for proper functioning)
    if (!terrainRenderer.Initialize())
    {
        Logger::Get().Write("ERROR: Failed to initialize TerrainRenderer\n");
        return false;
    }

    if (!bulletRenderer.Initialize())
    {
        Logger::Get().Write("ERROR: Failed to initialize BulletRenderer\n");
        return false;
    }

    if (!effectRenderer.Initialize())
    {
        Logger::Get().Write("ERROR: Failed to initialize EffectRenderer\n");
        return false;
    }

    if (!itemRenderer.Initialize())
    {
        Logger::Get().Write("ERROR: Failed to initialize ItemRenderer\n");
        return false;
    }

    if (!tankRenderer.Initialize())
    {
        Logger::Get().Write("ERROR: Failed to initialize TankRenderer\n");
        return false;
    }

    // Initialize new centralized rendering pipeline
    InitializeNewRenderingPipeline();

    Logger::Get().Write("GraphicsTask::Started\n");
    Logger::Get().Write("Renderer mode: %s\n", RendererMode::Name());
    return renderingPipeline != nullptr;
}

void GraphicsTask::Stop()
{
    // Phase 4: Cleanup new rendering pipeline first
    CleanupNewRenderingPipeline();

    // Cleanup legacy renderers
    terrainRenderer.Cleanup();
    bulletRenderer.Cleanup();
    effectRenderer.Cleanup();
    itemRenderer.Cleanup();
    tankRenderer.Cleanup();

    Logger::Get().Write("GraphicsTask: Stopped \n");
}

void GraphicsTask::Update()
{
    if (!App::GetSingleton().videoTask->RefreshDrawableSize()) return;
    
    // Get current player count from PlayerManager (via GameTask)
    int numPlayers = (App::GetSingleton().gameTask && App::GetSingleton().gameTask->GetPlayerManager()) 
        ? App::GetSingleton().gameTask->GetPlayerManager()->GetNumPlayers() 
        : 1;

    const int numViews = numPlayers > 1 ? 2 : 1;
    if (viewportManager.GetNumViewports() != numViews ||
        viewportManager.GetScreenWidth() != VideoTask::scrWidth ||
        viewportManager.GetScreenHeight() != VideoTask::scrHeight)
    {
        // Reconfigure for player-count, window, canvas or drawable DPI changes.
        if (renderingPipeline)
        {
            renderingPipeline->ConfigureViewports(numPlayers, VideoTask::scrWidth, VideoTask::scrHeight);
        }
        else
        {
            // Fallback viewport setup for legacy pipeline
            if (numPlayers > 1)
            {
                viewportManager.SetupSplitScreen(numPlayers, VideoTask::scrWidth, VideoTask::scrHeight);
            }
            else
            {
                viewportManager.SetupSinglePlayer(VideoTask::scrWidth, VideoTask::scrHeight);
            }

            float ratio = static_cast<float>(VideoTask::scrWidth) / static_cast<float>(VideoTask::scrHeight);
#ifdef __EMSCRIPTEN__
            RenderContext::Current().SetProjection(
                Matrix4::Perspective(45.0f, ratio, 0.1f, 1024.0f));
#else
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluPerspective(45.0, ratio, 0.1, 1024.0);
#endif
        }

    }

    // Basic desktop compatibility state. Browser transforms and colors are
    // supplied through RenderContext to the migrated shader-backed paths.
#ifndef __EMSCRIPTEN__
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);

    glDisable(GL_LIGHTING);
#endif

    if (App::GetSingleton().gameTask->IsGameStarted())
    {
        // Update cameras using CameraManager with player tanks from PlayerManager
        auto playerTanks = App::GetSingleton().gameTask->GetPlayerManager()->GetPlayerTanks();
        int numPlayers = App::GetSingleton().gameTask->GetPlayerManager()->GetNumPlayers();
        cameraManager.UpdateCamerasFromPointers(playerTanks, numPlayers, true);

        // Copy camera data to the old cams array for backward compatibility
        for (int i = 0; i < numPlayers && i < 4; i++)
        {
            const Camera &managedCam = cameraManager.GetCamera(i);
            cams[i].SetPos(managedCam.xpos(), managedCam.ypos(), managedCam.zpos());
            cams[i].SetFocus(managedCam.xfocus(), managedCam.yfocus(), managedCam.zfocus());
        }
    }

    // Use centralized rendering pipeline
    RenderWithNewPipeline();
}

// ============================================================================
// Phase 4: Centralized Rendering Pipeline Implementation
// ============================================================================

void GraphicsTask::InitializeNewRenderingPipeline()
{
    Logger::Get().Write("GraphicsTask: Initializing new centralized rendering pipeline\n");

    try
    {
        if (!resourceManager || !resourceManager->IsInitialized())
            throw std::runtime_error("renderer resource catalogue is not initialized");

        // Create SceneDataBuilder with references to game handlers
        // Note: GameWorld is nullptr at this point (GameTask starts after GraphicsTask)
        // Will be set via SetGameWorld() once GameWorld is initialized
        sceneDataBuilder = std::make_unique<SceneDataBuilder>(
            TankHandler::GetSingleton(),
            LevelHandler::GetSingleton(),
            gameWorld,
            App::GetSingleton().gameTask ? App::GetSingleton().gameTask->GetPlayerManager() : nullptr);

        // Create RenderingPipeline with references to managers
        renderingPipeline = std::make_unique<RenderingPipeline>(
            viewportManager,
            cameraManager,
            *resourceManager);

        if (!renderingPipeline->Initialize())
        {
            Logger::Get().Write("ERROR: Failed to initialize RenderingPipeline\n");
            renderingPipeline->Cleanup();
            renderingPipeline.reset();
            return;
        }

        Logger::Get().Write("GraphicsTask: New rendering pipeline initialized successfully\n");
    }
    catch (const std::exception &e)
    {
        Logger::Get().Write("ERROR: Exception during rendering pipeline initialization: %s\n", e.what());
        if (renderingPipeline) renderingPipeline->Cleanup();
        renderingPipeline.reset();
    }
}

void GraphicsTask::SetGameWorld(GameWorld* world)
{
    // Set GameWorld pointer and recreate SceneDataBuilder with valid GameWorld
    // Called by GameTask after GameWorld is initialized
    Logger::Get().Write("GraphicsTask: Setting GameWorld and recreating SceneDataBuilder\n");
    
    gameWorld = world;
    
    sceneDataBuilder = std::make_unique<SceneDataBuilder>(
        TankHandler::GetSingleton(),
        LevelHandler::GetSingleton(),
        gameWorld,
        App::GetSingleton().gameTask ? App::GetSingleton().gameTask->GetPlayerManager() : nullptr);
}

void GraphicsTask::CleanupNewRenderingPipeline()
{
    Logger::Get().Write("GraphicsTask: Cleaning up new rendering pipeline\n");

    if (renderingPipeline)
    {
        renderingPipeline->Cleanup();
        renderingPipeline.reset();
    }

    if (resourceManager)
    {
        resourceManager->Cleanup();
        resourceManager.reset();
    }

    // SceneDataBuilder doesn't need explicit cleanup
    sceneDataBuilder.reset();

    Logger::Get().Write("GraphicsTask: New rendering pipeline cleanup complete\n");
}

void GraphicsTask::RenderWithNewPipeline()
{
    if (!sceneDataBuilder || !renderingPipeline)
    {
        Logger::Get().Write("ERROR: New rendering pipeline not properly initialized!\n");
        return;
    }

    try
    {
        // Build scene data from current game state
        SceneData sceneData = sceneDataBuilder->BuildScene();
        
        // Extract camera data for split-screen support
        sceneData.cameras = sceneDataBuilder->ExtractCameraData();
        
        // Add game state information
        sceneData.gameStarted = App::GetSingleton().gameTask->IsGameStarted();
        sceneData.paused = App::GetSingleton().gameTask->IsPaused();
        sceneData.debugMode = App::GetSingleton().gameTask->IsDebugMode();

        // Render the complete scene using the centralized pipeline
        renderingPipeline->RenderAllPlayerViews(sceneData);
        
        // UI rendering is now handled by the RenderingPipeline
        // All HUD, Menu, and Debug rendering is data-driven and centralized
    }
    catch (const std::exception &e)
    {
        Logger::Get().Write("ERROR: Exception during centralized rendering: %s\n", e.what());
    }
}

void GraphicsTask::DrawSky()
{
    // Unused for now. Simple skybox rendering for level 48
}
