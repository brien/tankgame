//
//  GraphicsTask.h
//  tankgame
//
//

#pragma once

#ifdef _WIN32
    // If building in windows:
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
    // If building on macOS:
#include <OpenGL/gl.h>
#else
    // If building on Linux:
#include <GL/gl.h>
#endif

#include <SDL2/SDL_ttf.h>
#include <memory>

#include "ITask.h"
#include "igtl_qmesh.h"
#include "Tank.h"
#include "Camera.h"
#include "DisplayList.h"
#include "TextureHandler.h"
#include "rendering/ViewportManager.h"
#include "rendering/CameraManager.h"
#include "rendering/TerrainRenderer.h"
#include "rendering/BulletRenderer.h"
#include "rendering/EffectRenderer.h"
#include "rendering/ItemRenderer.h"
#include "rendering/TankRenderer.h"
#include "rendering/BulletDataExtractor.h"
#include "rendering/EffectDataExtractor.h"
#include "rendering/ItemDataExtractor.h"
#include "rendering/TankDataExtractor.h"
#include "VideoTask.h"
#include "rendering/ResourceManager.h"
#include "rendering/SceneDataBuilder.h"
#include "rendering/RenderingPipeline.h"

class GraphicsTask : public ITask
{
public:
    GraphicsTask();
    ~GraphicsTask() = default;

    TextureHandler textureHandler;
    TTF_Font *defaultFont;
    
    Camera cams[4];
    ViewportManager viewportManager;  // Manages viewport layout for split-screen
    CameraManager cameraManager;      // Manages camera positioning and behavior
    
    // Phase 4: New Centralized Rendering Pipeline Components
    std::unique_ptr<ResourceManager> resourceManager;      // Centralized resource management
    std::unique_ptr<SceneDataBuilder> sceneDataBuilder;    // Scene data extraction and building
    std::unique_ptr<RenderingPipeline> renderingPipeline;  // Centralized rendering orchestration
    
    // Legacy specialized renderers (will be managed by RenderingPipeline)
    TerrainRenderer terrainRenderer;  // Handles all terrain rendering
    BulletRenderer bulletRenderer; // Handles all bullet rendering
    EffectRenderer effectRenderer;    // Handles all visual effect rendering
    ItemRenderer itemRenderer;       // Handles all item/power-up rendering
    TankRenderer tankRenderer;     // Handles all tank rendering
    
    DisplayList cubelist1 = 1;
    DisplayList cubelist2;
    DisplayList bulletlist;
    DisplayList bodylist;
    DisplayList turretlist;
    DisplayList cannonlist;
    
    DisplayList itemlist;
    
    DisplayList squarelist;
    DisplayList squarelist2;
    
    DisplayList bodylistEx;
    DisplayList turretlistEx;
    DisplayList cannonlistEx;
    
    DisplayList bodylistEx2;
    DisplayList turretlistEx2;
    DisplayList cannonlistEx2;
    
    void DrawHUD(Tank& player);
    void DrawMenu(int option);
    void DrawSky();
    void DrawTextTest();
    void RenderText(const TTF_Font* Font, const GLubyte& R, const GLubyte& G, const GLubyte& B, const double& X, const double& Y, const double& Z, const char* Text);
    
    // Centralized rendering methods
    void InitializeNewRenderingPipeline();
    void CleanupNewRenderingPipeline();
    void RenderWithNewPipeline();          // Main centralized rendering method
    
    bool Start();
    void Update();
    void Stop();
    
    bool drawHUD = true;
    bool drawMenu = false;
private:
    igtl_QGLMesh bodymesh;
    igtl_QGLMesh turretmesh;
    igtl_QGLMesh cannonmesh;
    igtl_QGLMesh itemmesh;

    void BuildDisplayLists();
    void FixMesh(igtl_QGLMesh& mesh);
    void PrepareMesh(igtl_QGLMesh& mesh, const char* fileName);
    void RenderLegacyUIElements();  // Legacy HUD/UI rendering
};
