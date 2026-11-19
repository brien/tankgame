//
//  GraphicsTask.cpp
//  tankgame
//
//

#include "rendering/PlatformGL.h"

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
    const float ratio = viewportManager.GetViewport(0).GetAspectRatio();
    RenderContext::Current().SetProjection(Matrix4::Perspective(45.0f, ratio, 0.1f, 1024.0f));

    // Enable essential OpenGL features
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    // The GL context is current before the authoritative catalogue performs
    // any texture, buffer, shader, or compatibility display-list upload.
    resourceManager = std::make_unique<ResourceManager>();
    if (!resourceManager->Initialize())
    {
        Logger::Get().Write("ERROR: Failed to initialize renderer resources\n");
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
            RenderContext::Current().SetProjection(
                Matrix4::Perspective(45.0f, ratio, 0.1f, 1024.0f));
        }

    }

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
