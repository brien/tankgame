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
#include <stdlib.h>
#include <sys/types.h>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "App.h"
#include "TankHandler.h"
#include "BulletHandler.h"
#include "FXHandler.h"
#include "VideoTask.h"
#include "math.h"

typedef unsigned short WORD;
typedef unsigned char byte;


GraphicsTask::GraphicsTask()
{
    TTF_Init();
    const char* fontFilePath = "./fonts/DroidSansMono.ttf";
    defaultFont = TTF_OpenFont(fontFilePath, 256);
    if (!defaultFont)
    {
        printf("Unable to open font \n");
        Logger::Get().Write("GraphicsTask: failed loading file: %s  \n", fontFilePath);
        exit(1);
    }
}


bool GraphicsTask::Start()
{
    Logger::Get().Write("GraphicsTask::Starting\n");

    //glShadeModel( GL_SMOOTH );
    glClearColor( 0, 0, 0.0f, 0.0f );
    
    // Initialize viewport manager
    viewportManager.SetupSinglePlayer(VideoTask::scrWidth, VideoTask::scrHeight);
    
    // Setup our viewport.
    glViewport( 0, 0, VideoTask::scrWidth, VideoTask::scrHeight );
    
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    float ratio = static_cast<float>(VideoTask::scrWidth) / static_cast<float>(VideoTask::scrHeight);
    gluPerspective( 45.0, ratio, 0.1, 1024.0 );
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    glFrontFace(GL_CW);
    
    // Removed unused LightAmbient and LightSpecular variables to fix -Wunused-variable warnings
    GLfloat LightDiffuse[]= { 1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat LightPosition[]= { 0.0f, 1.0f, 0.0f, 0.0f };
    
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
    //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightAmbient);
    
    glPushMatrix();
    
    glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
    
    glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.1f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.05f);
    
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    glEnable(GL_TEXTURE_2D);
    
    textureHandler.LoadTextures();
    
    PrepareMesh(bodymesh, "nowbody.gsm");
    PrepareMesh(turretmesh, "nowturret.gsm");
    PrepareMesh(cannonmesh, "cannon.gsm");
    PrepareMesh(itemmesh, "body.gsm");
    
    //BuildDisplayLists();

    Logger::Get().Write("GraphicsTask::Started\n");

    bodymesh.SaveOBJ("converted_nowbody.obj");
    turretmesh.SaveOBJ("converted_nowturret.obj");
    itemmesh.SaveOBJ("converted_body.obj");
    
    if (bodymesh.LoadOBJ("converted_nowbody.obj")) {
    std::cout << "converted_nowbody OBJ file loaded successfully.\n";
    } else {
        std::cerr << "Failed to load OBJ file.\n";
    }

    if (bodymesh.SaveOBJ("converted_nowbody_testsave.obj")) {
    std::cout << "converted_nowbody_testsave OBJ file saved successfully.\n";
    } else {
        std::cerr << "Failed to save OBJ file.\n";
    }

    BuildDisplayLists();

    // Initialize new rendering pipeline components
    if (!terrainRenderer.Initialize()) {
        Logger::Get().Write("ERROR: Failed to initialize TerrainRenderer\n");
        return false;
    }
    
    if (!bulletRenderer.Initialize()) {
        Logger::Get().Write("ERROR: Failed to initialize NewBulletRenderer\n");
        return false;
    }
    
    if (!effectRenderer.Initialize()) {
        Logger::Get().Write("ERROR: Failed to initialize EffectRenderer\n");
        return false;
    }

    return true;
}

void GraphicsTask::FixMesh(igtl_QGLMesh& mesh)
{
    mesh.SafetyCheck();
    mesh.Unitize(.3);
    mesh.GenerateFacets();
    mesh.MergeVerticies();
    mesh.GenerateEdges();
}

void GraphicsTask::Stop()
{
    Logger::Get().Write("GraphicsTask: ClosingFont \n");
    TTF_CloseFont(defaultFont);
    
    // Cleanup new rendering pipeline
    terrainRenderer.Cleanup();
    bulletRenderer.Cleanup();
    effectRenderer.Cleanup();
    
    Logger::Get().Write("GraphicsTask: Stopped \n");
}



void GraphicsTask::Update()
{
    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    static int lastnumPlayers=TankHandler::GetSingleton().numPlayers;
    
    if(lastnumPlayers!=TankHandler::GetSingleton().numPlayers)
    {
        // Setup viewports when player count changes
        int numPlayers = TankHandler::GetSingleton().numPlayers;
        if (numPlayers > 1) {
            viewportManager.SetupSplitScreen(numPlayers, VideoTask::scrWidth, VideoTask::scrHeight);
        } else {
            viewportManager.SetupSinglePlayer(VideoTask::scrWidth, VideoTask::scrHeight);
        }
        
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity( );
        float ratio = static_cast<float>(VideoTask::scrWidth) / static_cast<float>(VideoTask::scrHeight/2);
        gluPerspective( 45.0, ratio, 0.1, 1024.0 );
        
        lastnumPlayers = numPlayers;
    }


    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );
    
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    
    
    glDisable(GL_LIGHTING);
    
    if(App::GetSingleton().gameTask->IsGameStarted())
    {
        // Update cameras using CameraManager instead of manual calculations
        int numPlayers = TankHandler::GetSingleton().numPlayers;
        cameraManager.UpdateCameras(TankHandler::GetSingleton().players, numPlayers, true);
        
        // Copy camera data to the old cams array for backward compatibility
        for(int i = 0; i < numPlayers && i < 4; i++)
        {
            const Camera& managedCam = cameraManager.GetCamera(i);
            cams[i].SetPos(managedCam.xpos(), managedCam.ypos(), managedCam.zpos());
            cams[i].SetFocus(managedCam.xfocus(), managedCam.yfocus(), managedCam.zfocus());
        }
    }
    
    if(TankHandler::GetSingleton().numPlayers>1)
    {
        // Render for Player 1 (camera index 1)
        viewportManager.SetActiveViewport(1);
        
        gluLookAt(cams[1].xpos(),cams[1].ypos(),cams[1].zpos(),
                  cams[1].xfocus(),cams[1].yfocus(),cams[1].zfocus(),
                  0, 1, 0);
        
        // Use new terrain renderer
        TerrainRenderData terrainData;
        LevelHandler::GetSingleton().populateTerrainRenderData(terrainData);
        terrainRenderer.RenderTerrain(terrainData);
        
        // Use new bullet rendering pipeline
        RenderBulletsWithNewPipeline();
        
        // Use new effect rendering pipeline
        RenderEffectsWithNewPipeline();
        
        // Use new item rendering pipeline
        RenderItemsWithNewPipeline();
        
        TankHandler::GetSingleton().DrawTanks();
        
        if(drawHUD)
        {
            DrawHUD(TankHandler::GetSingleton().players[1]);
        }
        
        // Render for Player 0 (camera index 0)
        viewportManager.SetActiveViewport(0);
        
        glLoadIdentity();
        
        glDisable(GL_LIGHTING);
        
    }
    
    gluLookAt(cams[0].xpos(),cams[0].ypos(),cams[0].zpos(),
              cams[0].xfocus(),cams[0].yfocus(),cams[0].zfocus(),
              0, 1, 0);
    
    if(LevelHandler::GetSingleton().levelNumber == 48)
    {
        DrawSky();
    }
    
    // Use new terrain renderer for main player view
    TerrainRenderData terrainData;
    LevelHandler::GetSingleton().populateTerrainRenderData(terrainData);
    terrainRenderer.RenderTerrain(terrainData);
    
    // Use new bullet rendering pipeline
    RenderBulletsWithNewPipeline();
    
    // Use new effect rendering pipeline
    RenderEffectsWithNewPipeline();
    
    // Use new item rendering pipeline
    RenderItemsWithNewPipeline();
    
    TankHandler::GetSingleton().DrawTanks();
    
    
    if(drawHUD)
    {
        DrawHUD(TankHandler::GetSingleton().players[0]);
    }
    
    if(drawMenu)
    {
        DrawMenu(App::GetSingleton().gameTask->GetMenuState());
    }

    //DrawTextTest();
    
    if (App::GetSingleton().gameTask->IsDebugMode())
    {
        char buffer[32];
        float framesPerSecond = 1.0f / GlobalTimer::dT;

        sprintf(buffer, "FPS: %.2f", framesPerSecond);
        RenderText(defaultFont, 255, 255, 255, 0.0, 0.0, 0.0, buffer);
    }
     //glBegin(GL_QUADS);
     //glTexCoord2i(1, 1);
     //glVertex3f(0,0,0);
     //
     //glTexCoord2i(1, 0);
     //glVertex3f(TankHandler::GetSingleton().players[0].x,0,0);
     //
     //glTexCoord2i(0, 0);
     //glVertex3f(TankHandler::GetSingleton().players[0].x,0,1);
     //
     //glTexCoord2i(0, 1);
     //glVertex3f(0,0,TankHandler::GetSingleton().players[0].x);
     //glEnd();
    
}


void GraphicsTask::PrepareMesh(igtl_QGLMesh& mesh, const char* fileName)
{
    FILE* tload = fopen(fileName, "rb");

    if (tload != NULL)
    {
        mesh.LoadGSM(tload);
        fclose(tload);
    }
}

void GraphicsTask::DrawSky()
{
    static float rot=0;
    
    rot+=10*GlobalTimer::dT;
    
    if(rot>360)
    {
        rot=0;
    }
    
    glDisable(GL_CULL_FACE);
    //glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    
    /*
     for(int i=0; i<64; i++)
     {
     glPushMatrix();
     glTranslatef(128*i, -64, 0);
     glScalef(50,50,50);
     glBindTexture(GL_TEXTURE_2D, textureArray[11]);
     glCallList(cubelist1);
     glPopMatrix();
     }
     
     for(int j=0; j<64; j++)
     {
     glPushMatrix();
     glTranslatef(0, -25, 128*j);
     glScalef(50,50,50);
     glBindTexture(GL_TEXTURE_2D, textureArray[11]);
     glCallList(cubelist1);
     glPopMatrix();
     }
     
     */
    
    glPushMatrix();
    glTranslatef(400, rot/2 - 50, 400);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TEXTURE_BLACK_CUBE]);
    cubelist1.Call(0);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(400, 10, 200);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TEXTURE_BLACK_CUBE]);
    cubelist1.Call(0);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-400, 50, -500);
    glRotatef(TankHandler::GetSingleton().closest, 0, 0, 1);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TEXTURE_BLACK_CUBE]);
    cubelist1.Call(0);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-200, 40, 600);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TEXTURE_WHITE_CUBE]);
    cubelist1.Call(0);
    glPopMatrix();
    
    glPushMatrix();
    
    glTranslatef(-100, 40, -500);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TEXTURE_BLACK_CUBE]);
    cubelist1.Call(0);
    glPopMatrix();
    
    glPushMatrix();
    glColor3f(0,1,0);
    glTranslatef(300, rot/2 - 50, -500);
    glRotatef(TankHandler::GetSingleton().closest, 0, 1, 0);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TEXTURE_BLACK_CUBE]);
    cubelist1.Call(0);
    glPopMatrix();
    
    glPushMatrix();
    glColor3f(0,1,0);
    glTranslatef(64, 30, -500);
    glRotatef(TankHandler::GetSingleton().closest, 0, 1, 0);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TEXTURE_BLACK_CUBE]);
    cubelist1.Call(0);
    glPopMatrix();
    
    glPushMatrix();
    glColor3f(1,0,1);
    glTranslatef(800, 30, 90);
    glRotatef(rot, 1, 0, 1);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TEXTURE_BLACK_CUBE]);
    cubelist1.Call(0);
    glPopMatrix();
    
    glPushMatrix();
    glColor3f(0,1,0);
    glTranslatef(-80, 2, -900);
    glRotatef(TankHandler::GetSingleton().closest, 0, 0, 0);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TEXTURE_BLACK_CUBE]);
    cubelist1.Call(0);
    glPopMatrix();
    
    glPushMatrix();
    glColor3f(1,0,1);
    glTranslatef(80, -20, 450);
    glRotatef(rot, 1, 0, 0);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TEXTURE_BLACK_CUBE]);
    cubelist1.Call(0);
    glPopMatrix();
    
    glPushMatrix();
    glColor3f(1,1,1);
    glTranslatef(900, 20, 600);
    glRotatef(rot, 1, 0, 1);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TEXTURE_BLACK_CUBE]);
    cubelist1.Call(0);
    glPopMatrix();
    
    glPushMatrix();
    glColor3f(1,1,1);
    glTranslatef(-900, 30, 60);
    glRotatef(rot, 1, 0, 1);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TEXTURE_BLACK_CUBE]);
    cubelist1.Call(0);
    glPopMatrix();
    
    
    glPushMatrix();
    glColor3f(1,1,1);
    glTranslatef(80, 15, -600);
    glRotatef(rot, 1, 0, 1);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TEXTURE_BLACK_CUBE]);
    cubelist1.Call(0);
    glPopMatrix();
    
    /*
     
     glPushMatrix();
     glColor3f(1,1,1);
     glTranslatef(0, 0, 0);
     glRotatef(rot, 1, 0, 1);
     glScalef(500,500,500);
     glBindTexture(GL_TEXTURE_2D, textureArray[11]);
     glCallList(cubelist1);
     glPopMatrix();
     
     */
    
    
    glEnable(GL_CULL_FACE);
    //glEnable(GL_DEPTH_TEST);
}

void GraphicsTask::RenderBulletsWithNewPipeline() {
    // Extract bullet render data from BulletHandler
    const std::vector<Bullet>& bullets = BulletHandler::GetSingleton().GetBullets();
    std::vector<BulletRenderData> bulletRenderData = BulletDataExtractor::ExtractBulletRenderData(bullets);
    
    // Render using the new bullet renderer
    bulletRenderer.RenderBullets(bulletRenderData);
}

void GraphicsTask::RenderEffectsWithNewPipeline() {
    // Extract effect render data from FXHandler
    const std::vector<FX>& effects = FXHandler::GetSingleton().fx;
    std::vector<EffectRenderData> effectRenderData = EffectDataExtractor::ExtractEffectRenderData(effects);
    
    // Render using the new effect renderer
    effectRenderer.RenderEffects(effectRenderData);
}

void GraphicsTask::RenderItemsWithNewPipeline() {
    // Extract item render data from LevelHandler
    const std::vector<Item>& items = LevelHandler::GetSingleton().items;
    std::vector<ItemRenderData> itemRenderData = ItemDataExtractor::ExtractRenderData(items);
    
    // Render using the new item renderer
    itemRenderer.RenderItems(itemRenderData);
}


void GraphicsTask::DrawHUD(Tank& player)
{
    static int times2=1;
    
    if(times2>60000)
    {
        times2=1;
    }
    
    times2*=2;
    
    glPushMatrix();
    
    glDisable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_TEXTURE_2D);
    
    glColor3f(1.0f,0.0f,0.0f);
    
    glEnable(GL_LINE_STIPPLE);
    glLineStipple( 16,  times2 );
    glBegin(GL_LINE_LOOP);
    
    glVertex3f(player.x,player.y+.1,player.z);
    glColor3f(0.0f,0.0f,0.0f);
    glVertex3f(player.x + 32*static_cast<float>(cos((player.ry+player.rty)*DTR)),player.y+.1, player.z + 32*static_cast<float>(sin((player.ry+player.rty)*DTR)));
    
    glEnd();
    
    glPopMatrix();
    
    glDisable(GL_LINE_STIPPLE);
    
    //glPushMatrix();
    
    glDisable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    //Treat 3D like 2D
    glTranslated(0,0,-1);
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    char buffer[32];
    float framesPerSecond = 1.0f / GlobalTimer::dT;

    sprintf(buffer, "FPS: %.2f", framesPerSecond);
    
    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TEXTURE_HEART]);
    glColor4f(1.0f,0.6,0.6f,1.0f);
    
    //Armor
    //Reload
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1);
    glVertex3f(-0.55f,0.37f,static_cast<float>(0));
    
    glTexCoord2f(1, 1);
    glVertex3f(-0.55f+0.03f,0.37f,static_cast<float>(0));
                
    glTexCoord2f(1, 0);
    glVertex3f(-0.55f+0.03f,0.34f,static_cast<float>(0));
    
    glTexCoord2f(0, 0);
    glVertex3f(-0.55f,0.34f,static_cast<float>(0));
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TEXTURE_BANG]);
    
    glColor4f(0.6f,0.6,1.0f,1.0f);
    
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1);
    glVertex3f(-0.55f,0.32f,static_cast<float>(0));
    
    glTexCoord2f(1, 1);
    glVertex3f(-0.55f+0.03f,0.32f,static_cast<float>(0));
                
    glTexCoord2f(1, 0);
    glVertex3f(-0.55f+0.03f,0.29f,static_cast<float>(0));
    
    glTexCoord2f(0, 0);
    glVertex3f(-0.55f,0.29f,static_cast<float>(0));
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    
    glDisable(GL_BLEND);
    
    glBegin(GL_LINE_LOOP);
    
    //Armor
    //Reload
    
    glColor3f(1.0f,0.8f,0.8f);
    //glCallList(cubelist1);
    glVertex3f(-0.51f,0.37f,0);
    glVertex3f(-0.51f+0.29f,0.37f,0);
    glVertex3f(-0.51f+0.29f,0.34f,0);
    glVertex3f(-0.51f,0.34f,0);
    
    //Reload
    glEnd();
    glBegin(GL_LINE_LOOP);
    
    glColor3f(0.5f,1.0f,1.0f);
    
    glVertex3f(-0.51f,0.32f,0);
    glVertex3f(-0.51f+0.29f,0.32f,0);
    glVertex3f(-0.51f+0.29f,0.29f,0);
    glVertex3f(-0.51f,0.29f,0);
    
    glEnd();
    
    glBegin(GL_LINE_LOOP);
    
    glColor3f(1.0f,1.0f,0.0f);
    
    glVertex3f(-0.51f,0.37f,0);
    glVertex3f(-0.51f+0.10f,0.37f,0);
    glVertex3f(-0.51f+0.10f,0.34f,0);
    glVertex3f(-0.51f,0.34f,0);
    
    glEnd();
    
    glBegin(GL_LINES);
    
    glColor3f(0.5f,1.0f,1.0f);
    
    glVertex3f(-0.51f+0.29f*player.fireCost/player.maxCharge,0.32f,0);
    glVertex3f(-0.51f+0.29f*player.fireCost/player.maxCharge,0.29f,0);
    
    glEnd();
    
    //glPopMatrix();
    
    //glDisable(GL_TEXTURE_2D);
    
    
    
    glBegin(GL_QUADS);
    
    //Armor
    //Reload
    float amer=(static_cast<float>(player.energy)/player.maxEnergy);
    if(amer>1)
    {
        amer=1;
        //glColor3f(1,0,0);
    }
    glColor3f(1.0,0.4f*amer,0.6f*amer);
    
    //glCallList(cubelist1);
    glVertex3f(-0.51f,0.37f,0);
    glVertex3f(-0.51f+0.29f*amer,0.37f,0);
    glVertex3f(-0.51f+0.29f*amer,0.34f,0);
    glVertex3f(-0.51f,0.34f,0);
    
    glEnd();
    
    if(player.charge<player.fireCost)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
    
    glBegin(GL_QUADS);
    
    //Reload
    float rper=(static_cast<float>(player.charge)/player.maxCharge);
    
    if(rper>1)
        rper=1;
    
    glColor4f(0.5f,rper,1.0f,0.02f);
    
    glVertex3f(-0.51f,0.32f,0);
    glVertex3f(-0.51f+0.29f*rper,0.32f,0);
    glVertex3f(-0.51f+0.29f*rper,0.29f,0);
    glVertex3f(-0.51f,0.29f,0);
    
    if(App::GetSingleton().gameTask->IsDebugMode())
    {
        //RenderText(defaultFont, 255, 255, 255, 0.0, 0.0, 0.0, "Debug Mode");
        float test=(2*static_cast<float>(GlobalTimer::dT));
        //if(test>1)test=1;
        glColor3f(1.0f,test,1.0f);
        glVertex3f(0.50f,-0.30f,0);
        glVertex3f(0.50f,-0.30f+test,0);
        glVertex3f(0.50f+0.01f,-0.30f+test,0);
        glVertex3f(0.50f+0.01f,-0.30f,0);
    }
    
    glEnd();
    
    if(TankHandler::GetSingleton().special[(-1*player.id)-1]<player.fireCost/5)
        glEnable(GL_BLEND);
    
    glBegin(GL_QUADS);
    
    
    float test=TankHandler::GetSingleton().combo[(-1*player.id)-1];
    test=test/100;
    
    //if(test>1)test=1;
    glColor3f(1.0f,2*test,0.1f);
    
    glVertex3f(0.50f,-0.37f,0);
    glVertex3f(0.50f,-0.37f+test,0);
    glVertex3f(0.50f+0.01f,-0.37f+test,0);
    glVertex3f(0.50f+0.01f,-0.37f,0);
    
    
    
    float spec=TankHandler::GetSingleton().special[(-1*player.id)-1];
    spec=spec/100;
    
    glColor4f(0.5f,spec,1.0f,0.02f);
    
    glVertex3f(0.52f,-0.37f,0);
    glVertex3f(0.52f,-0.37f+spec,0);
    glVertex3f(0.52f+0.01f,-0.37f+spec,0);
    glVertex3f(0.52f+0.01f,-0.37f,0);
    
    glEnd();
    
    float costspec=(player.fireCost/500);
    
    int nspec=(int)(spec/costspec);
    
    glColor4f(1.0f,1.0,1.0f,1.0f);
    
    glBegin(GL_LINES);
    
    for(int i=1; i<=nspec; i++)
    {
        glVertex3f(0.52f,-0.37f+costspec*i,0);
        glVertex3f(0.52f+0.01f,-0.37f+costspec*i,0);
    }
    
    glVertex3f(0.52f,-0.37f,0);
    glVertex3f(0.52f+0.01f,-0.37f,0);
    
    glEnd();
    
    /*
     
     glPushMatrix();
     
     glLoadIdentity();
     glTranslatef(0.5,-0.35,-1.0);
     
     glColor3f(1.0,1.0,1.0);
     glEnable(GL_TEXTURE_2D);
     
     glEnable(GL_BLEND);
     
     
     glBindTexture(GL_TEXTURE_2D, textureArray[TankHandler::GetSingleton().comboNum[(-1*player.id)-1]%10 ]);
     
     
     glBegin(GL_QUADS);
     glTexCoord2f(0, 1);
     glVertex3f(static_cast<float>(0),static_cast<float>(.04),static_cast<float>(0));
     
     glTexCoord2f(1, 1);
     glVertex3f(static_cast<float>(.04),static_cast<float>(.04),static_cast<float>(0));
     
     glTexCoord2f(1, 0);
     glVertex3f(static_cast<float>(.04),static_cast<float>(0),static_cast<float>(0));
     
     glTexCoord2f(0, 0);
     glVertex3f(static_cast<float>(0),static_cast<float>(0),static_cast<float>(0));
     
     glEnd();
     
     //---------
     //if(TankHandler::GetSingleton().comboNum[(-1*player.id)-1]<9 )
     //{
     glTranslatef(-0.04,0.0,0.0);
     
     glBindTexture(GL_TEXTURE_2D, textureArray[(int)TankHandler::GetSingleton().comboNum[(-1*player.id)-1]/10 ]);
     
     glBegin(GL_QUADS);
     
           glTexCoord2f(0, 1);
           glVertex3f((float)0,(float).04,(float)0);
     
           glTexCoord2f(1, 1);
           glVertex3f((float).04,(float).04,(float)0);
     
           glTexCoord2f(1, 0);
           glVertex3f((float).04,(float)0,(float)0);
           
           glTexCoord2f(0, 0);
           glVertex3f((float)0,(float)0,(float)0);
     glEnd();
     //}
     
     glPopMatrix();
     */
    
    if(player.bonus>0)
    {
        
        glPushMatrix();
        
        glLoadIdentity();
        glTranslatef(-0.05,-0.05*player.bonusTime,-1.0);
        
        glColor3f(1.0,1.0,1.0);
        glEnable(GL_TEXTURE_2D);
        
        glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[player.bonus]);
        
        glEnable(GL_BLEND);
        
        glBegin(GL_QUADS);
        glTexCoord2f(0, 1);
        glVertex3f((float)0,(float).1,(float)0);
        
        glTexCoord2f(1, 1);
        glVertex3f((float).1,(float).1,(float)0);
        
        glTexCoord2f(1, 0);
        glVertex3f((float).1,(float)0,(float)0);
        
        glTexCoord2f(0, 0);
        glVertex3f((float)0,(float)0,(float)0);
        
        glEnd();
        
        glPopMatrix();
    }
    
    glPushMatrix();
    
    glLoadIdentity();
    glTranslatef(0.0,-0.25,-1.0);
    
    glColor4f(player.hitAlpha,player.hitAlpha,player.hitAlpha,player.hitAlpha);
    glEnable(GL_TEXTURE_2D);
    
    glEnable(GL_BLEND);
    
    
    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TankHandler::GetSingleton().hitCombo[(-1*player.id)-1]%10 ]);
    
    
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1);
    glVertex3f((float)0,(float).04,(float)0);
    
    glTexCoord2f(1, 1);
    glVertex3f((float).04,(float).04,(float)0);
                
    glTexCoord2f(1, 0);
    glVertex3f((float).04,(float)0,(float)0);
    
    glTexCoord2f(0, 0);
    glVertex3f((float)0,(float)0,(float)0);
    
    glEnd();
    
    //---------
    //if(TankHandler::GetSingleton().comboNum[(-1*player.id)-1]<9 )
    //{
    glTranslatef(-0.04,0.0,0.0);
    
    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[(int)TankHandler::GetSingleton().hitCombo[(-1*player.id)-1]/10 ]);
    
    glBegin(GL_QUADS);
    
    glTexCoord2f(0, 1);
    glVertex3f((float)0,(float).04,(float)0);
    
    glTexCoord2f(1, 1);
    glVertex3f((float).04,(float).04,(float)0);
    
    glTexCoord2f(1, 0);
    glVertex3f((float).04,(float)0,(float)0);
    
    glTexCoord2f(0, 0);
    glVertex3f((float)0,(float)0,(float)0);
       glEnd();
    //}
    
    glPopMatrix();
    
    
    
    /*
     
     
     float bangM=((float)player.dist/50);
     if(bangM>1)bangM=1;
     
     if(TankHandler::GetSingleton().tanks.size()>0)
     {
     glColor4f(1.0f,bangM,0.1f,0.0);
     
     glVertex3f(-0.51f,-0.30f,0);
     glVertex3f(-0.51f,-0.27f*bangM,0);
     glVertex3f(-0.51f+0.01f,-0.27f*bangM,0);
     glVertex3f(-0.51f+0.01f,-0.30f,0);
     
     if(App::GetSingleton().gameTask->debug)
     {
     float test=(2*(float)GlobalTimer::dT);
     //if(test>1)test=1;
     glColor3f(1.0f,test,1.0f);
     
     glVertex3f(0.50f,-0.30f,0);
     glVertex3f(0.50f,-0.30f+test,0);
     glVertex3f(0.50f+0.01f,-0.30f+test,0);
     glVertex3f(0.50f+0.01f,-0.30f,0);
     }
     }
     
     
     
     
     
     glEnable(GL_BLEND);
     glBlendFunc(GL_ONE, GL_ONE);
     
     if(TankHandler::GetSingleton().tanks.size()>0)
     {
     glPushMatrix();
     glLoadIdentity();
     glTranslatef(-0.51f,-0.32*bangM,-1);
     
     //glColor3f(1.0,1.0,1.0);
     glColor4f(1.0f,bangM,0.1f,1.0f);
     glEnable(GL_TEXTURE_2D);
     glBindTexture(GL_TEXTURE_2D, textureArray[13]);
     glBegin(GL_QUADS);
     glTexCoord2f(0, 1);
     glVertex3f((float)0,(float).1,(float)0);
     
     glTexCoord2f(1, 1);
     glVertex3f((float).1,(float).1,(float)0);
     
     glTexCoord2f(1, 0);
     glVertex3f((float).1,(float)0,(float)0);
     
     glTexCoord2f(0, 0);
     glVertex3f((float)0,(float)0,(float)0);
     glEnd();
     glPopMatrix();
     }
     */
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glColor4f(1.0f,1.0f,1.0f,1.0f);
    
    if(App::GetSingleton().gameTask->IsPaused())
    {
        glPushMatrix();
        glLoadIdentity();
        glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[18]);
        glTranslatef(-0.3,-0.1,-1.0);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 1);
        glVertex3f((float)0,(float).05,(float)0);
        
        glTexCoord2f(1, 1);
        glVertex3f((float).05,(float).05,(float)0);
        
        glTexCoord2f(1, 0);
        glVertex3f((float).05,(float)0,(float)0);
        
        glTexCoord2f(0, 0);
        glVertex3f((float)0,(float)0,(float)0);
        glEnd();
        glPopMatrix();
    }
    
    if( App::GetSingleton().gameTask->IsVersusMode() && (!TankHandler::GetSingleton().players[0].alive || !TankHandler::GetSingleton().players[1].alive) )
    {
        glPushMatrix();
        glLoadIdentity();
        glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[24]);
        glTranslatef(0.0,0.2,-1.0);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 1);
        glVertex3f((float)0,(float).2,(float)0);
        
        glTexCoord2f(1, 1);
        glVertex3f((float).2,(float).2,(float)0);
        
        glTexCoord2f(1, 0);
        glVertex3f((float).2,(float)0,(float)0);
        
        glTexCoord2f(0, 0);
        glVertex3f((float)0,(float)0,(float)0);
        glEnd();
        
        glTranslatef(0.08,-0.05, 0.0);
        
        glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TankHandler::GetSingleton().wins[(-1*player.id)-1]%10 ]);
        
        
        glBegin(GL_QUADS);
        glTexCoord2f(0, 1);
        glVertex3f((float)0,(float).08,(float)0);
        
        glTexCoord2f(1, 1);
        glVertex3f((float).08,(float).08,(float)0);
        
        glTexCoord2f(1, 0);
        glVertex3f((float).08,(float)0,(float)0);
        
        glTexCoord2f(0, 0);
        glVertex3f((float)0,(float)0,(float)0);
        
        glEnd();
        
        //---------
        glTranslatef(-0.08,0.0,0.0);
        
        glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[(int)TankHandler::GetSingleton().wins[(-1*player.id)-1]/10 ]);
        
        glBegin(GL_QUADS);
        
        glTexCoord2f(0, 1);
        glVertex3f((float)0,(float).08,(float)0);
        
        glTexCoord2f(1, 1);
        glVertex3f((float).08,(float).08,(float)0);
        
        glTexCoord2f(1, 0);
        glVertex3f((float).08,(float)0,(float)0);
        
        glTexCoord2f(0, 0);
        glVertex3f((float)0,(float)0,(float)0);
        glEnd();
        
        
        glPopMatrix();
    }
    
    
    
    
    glPushMatrix();
    
    glLoadIdentity();
    glTranslatef(-0.5,-0.35,-1.0);
    
    glColor3f(1.0,1.0,1.0);
    glEnable(GL_TEXTURE_2D);

    // Ones of enemy tanks left:
    glTranslatef(-0.04, 0.0, 0.0);

    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TankHandler::GetSingleton().tanks.size()%10]);
    
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1);
    glVertex3f((float)0,(float).04,(float)0);
    
    glTexCoord2f(1, 1);
    glVertex3f((float).04,(float).04,(float)0);
                
    glTexCoord2f(1, 0);
    glVertex3f((float).04,(float)0,(float)0);
    
    glTexCoord2f(0, 0);
    glVertex3f((float)0,(float)0,(float)0);
    glEnd();
    
    // Tens of enemy tanks left:
    glTranslatef(-0.04,0.0,0.0);
    
    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[(int)TankHandler::GetSingleton().tanks.size()/10 ]);
    
    glBegin(GL_QUADS);
    
    glTexCoord2f(0, 1);
    glVertex3f((float)0,(float).04,(float)0);
    
    glTexCoord2f(1, 1);
    glVertex3f((float).04,(float).04,(float)0);
    
    glTexCoord2f(1, 0);
    glVertex3f((float).04,(float)0,(float)0);
    
    glTexCoord2f(0, 0);
    glVertex3f((float)0,(float)0,(float)0);
    glEnd();

    // X:
    glTranslatef(-0.04, 0.0, 0.0);

    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TEXTURE_X]);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 1);
    glVertex3f((float)0, (float).04, (float)0);

    glTexCoord2f(1, 1);
    glVertex3f((float).04, (float).04, (float)0);

    glTexCoord2f(1, 0);
    glVertex3f((float).04, (float)0, (float)0);

    glTexCoord2f(0, 0);
    glVertex3f((float)0, (float)0, (float)0);
    glEnd();

    // EnemyTank Icon:
    glTranslatef(-0.07, -0.02, 0.0);

    glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[TEXTURE_ENEMY]);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 1);
    glVertex3f((float)0, (float).07, (float)0);

    glTexCoord2f(1, 1);
    glVertex3f((float).07, (float).07, (float)0);

    glTexCoord2f(1, 0);
    glVertex3f((float).07, (float)0, (float)0);

    glTexCoord2f(0, 0);
    glVertex3f((float)0, (float)0, (float)0);
    glEnd();
    
    glPopMatrix();
    
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
}

void GraphicsTask::DrawTextTest()
{
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    //Treat 3D like 2D
    glTranslated(0, 0, -1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    char buffer[32];

    sprintf(buffer, "F: %s", "string");
    //RenderText(defaultFont, 255, 255, 255, 0.0, 0.0, 0.0, buffer);

    glBegin(GL_QUADS);
    glTexCoord2d(0, 0);
    glVertex3f(0.40f, 0.37f, 0);
    glTexCoord2d(1, 0);
    glVertex3f(0.40f + 0.29f, 0.37f, 0);
    glTexCoord2d(1, 1);
    glVertex3f(0.40f + 0.29f, 0.30f, 0);
    glTexCoord2d(0, 1);
    glVertex3f(0.40f, 0.30f, 0);
    glEnd();


    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
}

void GraphicsTask::DrawMenu(int option)
{
    glPushMatrix();
    
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    //Treat 3D like 2D
    glTranslated(0,0,-1);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    
    if(option==0)
    {
        glBegin(GL_LINE_LOOP);
        
        
        
        glVertex3f(0.03f-0.55f,-0.06f-0.13f,0);
        glVertex3f(0.35f-0.55f,-0.06f-0.13f,0);
        glVertex3f(0.35f-0.55f,-0.15f-0.13f,0);
        glVertex3f(0.03f-0.55f,-0.15f-0.13f,0);
        
        glEnd();
        
        glColor4f(0.0f,0.4f,0.4f, 0.1f);
        
        glBegin(GL_QUADS);
        
        //glColor3f(0.0f,1.0f,1.0f);
        
        glVertex3f(0.03f-0.55f,-0.06f-0.13f,0);
        glVertex3f(0.35f-0.55f,-0.06f-0.13f,0);
        glVertex3f(0.35f-0.55f,-0.15f-0.13f,0);
        glVertex3f(0.03f-0.55f,-0.15f-0.13f,0);
        
        glEnd();
        
    }
    else
        if(option==1)
        {
            glBegin(GL_LINE_LOOP);
            
            glColor3f(0.0f,1.0f,1.0f);
            
            glVertex3f(0.03f,-0.06f-0.13f,0);
            glVertex3f(0.35f,-0.06f-0.13f,0);
            glVertex3f(0.35f,-0.15f-0.13f,0);
            glVertex3f(0.03f,-0.15f-0.13f,0);
            
            glEnd();
            
            glColor4f(0.0f,0.4f,0.4f, 0.1f);
            
            glBegin(GL_QUADS);
            
            //glColor3f(0.0f,1.0f,1.0f);
            
            glVertex3f(0.03f,-0.06f-0.13f,0);
            glVertex3f(0.35f,-0.06f-0.13f,0);
            glVertex3f(0.35f,-0.15f-0.13f,0);
            glVertex3f(0.03f,-0.15f-0.13f,0);
            
            glEnd();
            
        }
        else
            if(option==2)
            {
                glBegin(GL_LINE_LOOP);
                
                glColor3f(0.0f,1.0f,1.0f);
                
                glVertex3f(0.03f,-0.06f-0.23f,0);
                glVertex3f(0.35f,-0.06f-0.23f,0);
                glVertex3f(0.35f,-0.15f-0.23f,0);
                glVertex3f(0.03f,-0.15f-0.23f,0);
                
                glEnd();
                
                glColor4f(0.0f,0.4f,0.4f, 0.1f);
                
                glBegin(GL_QUADS);
                
                //glColor3f(0.0f,1.0f,1.0f);
                
                glVertex3f(0.03f,-0.06f-0.23f,0);
                glVertex3f(0.35f,-0.06f-0.23f,0);
                glVertex3f(0.35f,-0.15f-0.23f,0);
                glVertex3f(0.03f,-0.15f-0.23f,0);
                
                glEnd();
                
            }
    
    /*
     glEnable(GL_BLEND);
     glBlendFunc(GL_ONE, GL_ONE);
     
     glColor3f(1.0,1.0,1.0);
     glEnable(GL_TEXTURE_2D);
     
     glBindTexture(GL_TEXTURE_2D, textureArray[1]);
     
     glBegin(GL_QUADS);
     glTexCoord2f(0, 1);
     glVertex3f(-0.1f, 0.1f, 0.0f);
     
     glTexCoord2f(1, 1);
     glVertex3f(0.0f, 0.1f, 0.0f);
     
     glTexCoord2f(1, 0);
     glVertex3f(0.0f, 0.0f, 0.0f);
     
     glTexCoord2f(0, 0);
     glVertex3f(-0.1f, 0.0f, 0.0f);
     glEnd();
     
     glBindTexture(GL_TEXTURE_2D, textureArray[15]);
     
     glBegin(GL_QUADS);
     glTexCoord2f(0, 1);
     glVertex3f(-0.2f, 0.1f, 0.0f);
     
     glTexCoord2f(1, 1);
     glVertex3f(-0.1f, 0.1f, 0.0f);
     
     glTexCoord2f(1, 0);
     glVertex3f(-0.1f, 0.0f, 0.0f);
     
     glTexCoord2f(0, 0);
     glVertex3f(-0.2f, 0.0f, 0.0f);
     glEnd();*/
    
    glPopMatrix();
    
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
}

void GraphicsTask::RenderText(const TTF_Font* Font, const GLubyte& R, const GLubyte& G, const GLubyte& B, const double& X, const double& Y, const double& Z, const char* Text)
{
    GLenum error = glGetError();
    while (error != GL_NO_ERROR)
        error = glGetError();

    glPushMatrix();

    glDisable(GL_LIGHTING);
    

    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST);
    //Treat 3D like 2D
    glTranslated(0, 0, -1);

    glColor3f(1.0, 1.0, 1.0);
    //-----------
    /*Create some variables.*/
    SDL_Color Color = { 255, 255, 0 };
    SDL_Surface* Message = TTF_RenderText_Blended(const_cast<TTF_Font*>(defaultFont), Text, Color);
    unsigned Texture = 0;

    glGenTextures(1, &Texture);
    glBindTexture(GL_TEXTURE_2D, Texture); //textureHandler.GetTextureArray()[12]);
    //glBindTexture(GL_TEXTURE_2D, textureHandler.GetTextureArray()[12]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Message->w, Message->h, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, Message->pixels);

    error = glGetError();
    if (error == GL_INVALID_ENUM)
    {
        printf("Invalid Enum");exit(1);
    }

    /*Generate an OpenGL 2D texture from the SDL_Surface*.*/
    //glGenTextures(1, &Texture);
    //glBindTexture(GL_TEXTURE_2D, Texture);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    error = glGetError();
    while (error != GL_NO_ERROR)
        error = glGetError();

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Message->w, Message->h, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, Message->pixels);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Message->w, Message->h, 0, 0, GL_UNSIGNED_BYTE, Message->pixels);

    /*Draw this texture on a quad with the given xyz coordinates.*/
    glBegin(GL_QUADS);
    glTexCoord2d(0, 0);
    glVertex3f(0.40f, 0.37f, 0);
    glTexCoord2d(1, 0);
    glVertex3f(0.40f + 0.29f, 0.37f, 0);
    glTexCoord2d(1, 1);
    glVertex3f(0.40f + 0.29f, 0.30f, 0);
    glTexCoord2d(0, 1);
    glVertex3f(0.40f, 0.30f, 0);
    glEnd();

    /*Clean up.*/
    glDeleteTextures(1, &Texture);
    SDL_FreeSurface(Message);
    //-----------
    glPopMatrix();
}

void GraphicsTask::BuildDisplayLists()
{
    //cubelist1=glGenLists(32);
    //cubelist1 = 1;
    cubelist1 = DisplayList(1);
    
    //glNewList(cubelist1,GL_COMPILE);
    cubelist1.BeginNewList();
    
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.5f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f,  0.5f,  0.5f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f,  0.5f,  0.5f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f, -0.5f);	// Top Right Of The Texture and Quad
    // Bottom Face
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, -0.5f, -0.5f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f, -0.5f, -0.5f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);	// Bottom Right Of The Texture and Quad
    // Front Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f,  0.5f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f,  0.5f);	// Top Left Of The Texture and Quad
    // Back Face
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.5f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f,  0.5f, -0.5f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f, -0.5f);	// Bottom Left Of The Texture and Quad
    // Right face
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f, -0.5f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f, -0.5f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f,  0.5f,  0.5f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);	// Bottom Left Of The Texture and Quad
    // Left Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f,  0.5f,  0.5f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.5f);	// Top Left Of The Texture and Quad
    glEnd();
    
    //glEndList();
    cubelist1.EndNewList();
    
    //cubelist2 = 2;
    cubelist2 = DisplayList(1);
    
    float x=40;
    float y=1;
    float z=20;
    
    cubelist2.BeginNewList();
    //glNewList(cubelist2,GL_COMPILE);
    
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1*x,  y, -1*z);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1*x,  y,    z);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f(   x,  y,    z);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f(   x,  y, -1*z);	// Top Right Of The Texture and Quad
    // Bottom Face
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1*x, -1*y, -1*z);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( x, -1*y, -1*z);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( x, -1*y,  z);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1*x, -1*y,  z);	// Bottom Right Of The Texture and Quad
    // Front Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1*x, -1*y,  z);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f( x, -1*y,  z);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f( x,  y,  z);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1*x, y,  z);	// Top Left Of The Texture and Quad
    // Back Face
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1*x, -1*y, -1*z);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1*x,  y, -1*z);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( x,  y, -1*z);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( x, -1*y, -1*z);	// Bottom Left Of The Texture and Quad
    // Right face
    glTexCoord2f(1.0f, 0.0f); glVertex3f( x, -1*y, -1*z);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f( x,  y, -1*z);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( x,  y,  z);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( x, -1*y,  z);	// Bottom Left Of The Texture and Quad
    // Left Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1*x, -1*y, -1*z);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1*x, -1*y,  z);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1*x,  y,  z);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1*x,  y, -1*z);	// Top Left Of The Texture and Quad
    glEnd();
    
    //glEndList();
    cubelist2.EndNewList();
    
    bulletlist = DisplayList(1);
    bulletlist.BeginNewList();
    
    //glNewList(cubelist1+2,GL_COMPILE);
    
    //LevelHandler::GetSingleton().DrawTerrain();
    
    
    glBegin(GL_TRIANGLES);
    glNormal3f(0.000000,1.000000,0.000000);
    glTexCoord2f(0.000000,-0.707107);glVertex3f(-0.3,0.1,-0.025f);
    glTexCoord2f(0.000000,0.000000);glVertex3f(0.5,0.1,-0.025f);
    glTexCoord2f(1.000000,0.000000);glVertex3f(0.5,0.1,0.025f);
    glNormal3f(0.000000,1.000000,0.000000);
    glTexCoord2f(1.000000,1.000000);glVertex3f(-0.3,0.1,-0.025f);
    glTexCoord2f(0.000000,0.000000);glVertex3f(0.5,0.1,0.025f);
    glTexCoord2f(1.000000,1.000000);glVertex3f(-0.3,0.1,0.025f);
    glNormal3f(0.000000,-0.707107,-0.707107);
    glTexCoord2f(0.000000,1.000000);glVertex3f(-0.3,0.1,-0.025f);
    glTexCoord2f(0.000000,0.000000);glVertex3i(-0.3,0,0);
    glTexCoord2f(0.000000,1.000000);glVertex3i(0.5,0,0);
    glNormal3f(0.000000,-0.707107,-0.707107);
    glTexCoord2f(1.000000,1.000000);glVertex3f(-0.3,0.1,-0.025f);
    glTexCoord2f(0.000000,0.000000);glVertex3i(0.5,0,0);
    glTexCoord2f(1.000000,1.000000);glVertex3f(0.5,0.1,-0.025f);
    glNormal3f(-1.000000,0.000000,0.000000);
    glTexCoord2f(1.000000,0.000000);glVertex3f(-0.3,0.1,-0.025f);
    glTexCoord2f(0.000000,0.000000);glVertex3f(-0.3,0.1,0.025f);
    glTexCoord2f(0.000000,1.000000);glVertex3i(-0.3,0,0);
    glNormal3f(1.000000,0.000000,0.000000);
    glTexCoord2f(0.000000,1.000000);glVertex3f(0.5,0.1,-0.025f);
    glTexCoord2f(1.000000,0.000000);glVertex3i(0.5,0,0);
    glTexCoord2f(1.000000,0.000000);glVertex3f(0.5,0.1,0.025f);
    glNormal3f(0.000000,-0.707107,0.707107);
    glTexCoord2f(1.000000,1.000000);glVertex3f(-0.3,0.1,0.025f);
    glTexCoord2f(0.000000,1.000000);glVertex3f(0.5,0.1,0.025f);
    glTexCoord2f(1.000000,1.000000);glVertex3i(0.5,0,0);
    glNormal3f(0.000000,-0.707107,0.707107);
    glTexCoord2f(1.000000,1.000000);glVertex3f(-0.3,0.1,0.025f);
    glTexCoord2f(0.000000,1.000000);glVertex3i(0.5,0,0);
    glTexCoord2f(1.000000,1.000000);glVertex3i(-0.3,0,0);
    glEnd();
    
    
    //glEndList();
    bulletlist.EndNewList();
    
    
    //bodylistEx=cubelist1+3;
    bodylistEx = DisplayList(1);
    
    //glNewList(cubelist1+3, GL_COMPILE);
    bodylistEx.BeginNewList();

    bodymesh.DrawTrianglesExtruded(.01);
    
    //glEndList();
    bodylistEx.EndNewList();
    
    //turretlistEx=cubelist1+4;
    turretlistEx = DisplayList(1);
    
    //glNewList(cubelist1+4, GL_COMPILE);
    turretlistEx.BeginNewList();
    
    turretmesh.DrawTrianglesExtruded(.01);
    
    //glEndList();
    turretlistEx.EndNewList();
    
    //cannonlistEx=cubelist1+5;
    cannonlistEx = DisplayList(1);
    
    //glNewList(cubelist1+5, GL_COMPILE);
    cannonlistEx.BeginNewList();
    
    cannonmesh.DrawTrianglesExtruded(.01);
    
    //glEndList();
    cannonlistEx.EndNewList();
    
    //bodylist=cubelist1+6;
    bodylist = DisplayList(1);
    
    //glNewList(cubelist1+6, GL_COMPILE);
    bodylist.BeginNewList();
    
    bodymesh.DrawTriangles();
    
    //glEndList();
    bodylist.EndNewList();
    
    //turretlist=cubelist1+7;
    turretlist = DisplayList(1);
    
    //glNewList(cubelist1+7, GL_COMPILE);
    turretlist.BeginNewList();
    
    turretmesh.DrawTriangles();
    
    //glEndList();
    turretlist.EndNewList();
    
    //cannonlist=cubelist1+8;
    
    //glNewList(cubelist1+8, GL_COMPILE);
    cannonlist.BeginNewList();
    
    cannonmesh.DrawTriangles();
    
    //glEndList();
    cannonlist.EndNewList();
    
    //bodylistEx2=cubelist1+9;
    
    //glNewList(cubelist1+9, GL_COMPILE);
    bodylistEx2.BeginNewList();
    
    bodymesh.DrawEdgesExtruded(.01);
    
    //glEndList();
    bodylistEx2.EndNewList();
    
    //turretlistEx2=cubelist1+10;
    
    //glNewList(cubelist1+10, GL_COMPILE);
    turretlistEx2.BeginNewList();
    
    turretmesh.DrawEdgesExtruded(.01);
    
    //glEndList();
    turretlistEx2.EndNewList();
    
    //cannonlistEx2=cubelist1+11;
    cannonlistEx2 = DisplayList(1);
    
    //glNewList(cubelist1+11, GL_COMPILE);

    cannonlistEx2.BeginNewList();
    
    cannonmesh.DrawEdgesExtruded(.01);
    
    //glEndList();
    cannonlistEx2.EndNewList();
    
    //squarelist=cubelist1+12;
    squarelist = DisplayList(1);

    //glNewList(cubelist1+12, GL_COMPILE);
    squarelist.BeginNewList();
    
    glBegin(GL_QUADS);
    
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.0f, -0.5f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f, 0.0f, -0.5f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, 0.0f,  0.5f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, 0.0f,  0.5f);	// Bottom Right Of The Texture and Quad
    
    glEnd();
    
    //glEndList();
       squarelist.EndNewList();
    
    //squarelist2=cubelist1+13;
    squarelist2 = DisplayList(1);
    
    //glNewList(cubelist1+13, GL_COMPILE);
    squarelist2.BeginNewList();
    
    glBegin(GL_LINE_LOOP);
    
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.0f, -0.5f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f, 0.0f, -0.5f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, 0.0f,  0.5f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, 0.0f,  0.5f);	// Bottom Right Of The Texture and Quad
    
    glEnd();
    
    //glEndList();
    squarelist2.EndNewList();
     
    itemlist = DisplayList(1);
    itemlist.BeginNewList();
    
    //itemlist=cubelist1+14;
    
    //glNewList(cubelist1+14, GL_COMPILE);
    
    //glBegin(GL_LINE_LOOP);
    
    itemmesh.DrawTrianglesExtruded(.01);
    //itemmesh.DrawEdges();
    //
    ////glEnd();
    //
    //glEndList();
    itemlist.EndNewList();

    return;
}


