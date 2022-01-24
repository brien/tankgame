//
//  GraphicsTask.cpp
//  tankgame
//
//


#ifdef _WIN32
    // If building in windows:
    #pragma warning(disable : 4996)
    #include <windows.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#else
    //if not:
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#endif

#include "GraphicsTask.h"
#include <stdlib.h>
#include <sys/types.h>
#include <iostream>

#include "App.h"
#include "TankHandler.h"
#include "LevelHandler.h"
#include "FXHandler.h"
#include "VideoTask.h"

typedef unsigned short WORD;
typedef unsigned char byte;


GraphicsTask::GraphicsTask() : alphalist{ 0 }
{
    drawHUD=true;
    drawMenu=false;
    cubelist1=1;
    TTF_Init();
    const char* fontFilePath = "./fonts/DroidSansMono.ttf";
    defaultFont = TTF_OpenFont(fontFilePath, 256);
    if (!defaultFont)
    {
        printf("Unable to open font");
        Logger::Get().Write("GraphicsTask: failed loading file: %s  \n", fontFilePath);
        exit(1);
    } //The program exits here
}

GraphicsTask::~GraphicsTask()
{
    TTF_CloseFont(defaultFont);
}

void GraphicsTask::TGA_Texture(unsigned int textureArray[], const char* strFileName, int ID, bool wrap)
{
    if(!strFileName)
    {
        return;
    }
    
    tImageTGA *pBitMap = Load_TGA(strFileName);
    
    if(pBitMap == NULL)	return;
    
    glGenTextures(1, &textureArray[ID]);
    glBindTexture(GL_TEXTURE_2D, textureArray[ID]);
    int textureType = GL_RGB;
    if(pBitMap->channels == 4)	textureType = GL_RGBA;
    gluBuild2DMipmaps(GL_TEXTURE_2D, pBitMap->channels, pBitMap->size_x, pBitMap->size_y, textureType, GL_UNSIGNED_BYTE, pBitMap->data);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    if(!wrap)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    
    
    if (pBitMap)
    {
        if (pBitMap->data)
        {
            free(pBitMap->data);
        }
        free(pBitMap);
    }
}


tImageTGA *GraphicsTask::Load_TGA(const char *strfilename)
{
    tImageTGA *pImgData	= NULL;
    FILE *pFile			= NULL;
    WORD width			= 0;
    WORD height			= 0;
    byte length			= 0;
    byte imgType		= 0;
    byte bits			= 0;
    int channels		= 0;
    int stride			= 0;
    int i				= 0;
    
    
    if((pFile = fopen(strfilename, "rb")) == NULL)
    {
        std::cerr << "GraphicsTask:Load_TGA: Failed to open " << strfilename << std::endl;
        //GuiTask::RenderSpacedBitmapString(30,30,10,(void *)GuiTask::font,"ERROR");
        return NULL;
    }
    
    
    pImgData = (tImageTGA*)malloc(sizeof(tImageTGA));
    
    fread(&length, sizeof(byte), 1, pFile);
    
    fseek(pFile,1,SEEK_CUR);
    
    fread(&imgType, sizeof(byte), 1, pFile);
    
    fseek(pFile, 9, SEEK_CUR);
    
    fread(&width,  sizeof(WORD), 1, pFile);
    fread(&height, sizeof(WORD), 1, pFile);
    fread(&bits,   sizeof(byte), 1, pFile);
    
    fseek(pFile, length + 1, SEEK_CUR);
    
    if(imgType != TGA_RLE)
    {
        // Check for 24 or 32 Bit
        if(bits == 24 || bits == 32)
        {
            
            channels = bits / 8;
            stride = channels * width;
            pImgData->data = new unsigned char[stride * height];
            
            for(int y = 0; y < height; y++)
            {
                unsigned char *pLine = &(pImgData->data[stride * y]);
                
                fread(pLine, stride, 1, pFile);
                
                for(i = 0; i < stride; i += channels)
                {
                    int temp     = pLine[i];
                    pLine[i]     = pLine[i + 2];
                    pLine[i + 2] = temp;
                }
            }
        }
        
        // Check for 16 Bit
        else if(bits == 16)
        {
            unsigned short pixels = 0;
            int r=0, g=0, b=0;
            
            channels = 3;
            stride = channels * width;
            pImgData->data = new unsigned char[stride * height];
            
            for(int i = 0; i < width*height; i++)
            {
                fread(&pixels, sizeof(unsigned short), 1, pFile);
                
                b = (pixels & 0x1f) << 3;
                g = ((pixels >> 5) & 0x1f) << 3;
                r = ((pixels >> 10) & 0x1f) << 3;
                
                pImgData->data[i * 3 + 0] = r;
                pImgData->data[i * 3 + 1] = g;
                pImgData->data[i * 3 + 2] = b;
            }
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        byte rleID = 0;
        int colorsRead = 0;
        channels = bits / 8;
        stride = channels * width;
        
        pImgData->data = new unsigned char[stride * height];
        byte *pColors = new byte [channels];
        
        while(i < width*height)
        {
            
            fread(&rleID, sizeof(byte), 1, pFile);
            
            
            if(rleID < 128)
            {
                rleID++;
                
                while(rleID)
                {
                    fread(pColors, sizeof(byte) * channels, 1, pFile);
                    
                    pImgData->data[colorsRead + 0] = pColors[2];
                    pImgData->data[colorsRead + 1] = pColors[1];
                    pImgData->data[colorsRead + 2] = pColors[0];
                    
                    if(bits == 32)	pImgData->data[colorsRead + 3] = pColors[3];
                    
                    i++;
                    rleID--;
                    colorsRead += channels;
                }
            }
            else
            {
                rleID -= 127;
                
                fread(pColors, sizeof(byte) * channels, 1, pFile);
                
                while(rleID)
                {
                    pImgData->data[colorsRead + 0] = pColors[2];
                    pImgData->data[colorsRead + 1] = pColors[1];
                    pImgData->data[colorsRead + 2] = pColors[0];
                    
                    if(bits == 32)	pImgData->data[colorsRead + 3] = pColors[3];
                    
                    i++;
                    rleID--;
                    colorsRead += channels;
                }
            }
        }
    }
    
    fclose(pFile);
    
    
    pImgData->channels  = channels;
    pImgData->size_x    = width;
    pImgData->size_y    = height;
    
    return pImgData;
}


bool GraphicsTask::Start()
{
    Logger::Get().Write("GraphicsTask::Starting\n");

    //glShadeModel( GL_SMOOTH );
    glClearColor( 0, 0, 0.0f, 0.0f );
    
    // Setup our viewport.
    glViewport( 0, 0, VideoTask::scrWidth, VideoTask::scrHeight );
    
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    float ratio = (float) (VideoTask::scrWidth) / (float) (VideoTask::scrHeight);
    gluPerspective( 45.0, ratio, 0.1, 1024.0 );
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    glFrontFace(GL_CW);
    
    GLfloat LightAmbient[]= { 1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat LightDiffuse[]= { 1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat LightSpecular[]= { 0.8f, 0.8f, 0.8f, 0.8f};
    
    GLfloat LightPosition[]= { 0.0f, 1.0f, 0.0f, 0.0f };
    
    
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
    //	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightAmbient);
    
    glPushMatrix();
    
    glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
    
    glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.1);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.05);
    
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    glEnable(GL_TEXTURE_2D);
    
    TGA_Texture(textureArray, "texture/cube1.tga", TEXTURE_WHITE_CUBE, true);
    TGA_Texture(textureArray, "texture/cube2.tga", TEXTURE_BLACK_CUBE, true);
    TGA_Texture(textureArray, "texture/trail.tga", TEXTURE_EXIT, false);
    TGA_Texture(textureArray, "texture/bang.tga", TEXTURE_BANG, true);
    TGA_Texture(textureArray, "texture/x.tga", TEXTURE_X, true);
    TGA_Texture(textureArray, "texture/cube12.tga", TEXTURE_CHECKER, true);
    TGA_Texture(textureArray, "texture/heart.tga", TEXTURE_HEART, true);
    TGA_Texture(textureArray, "texture/p_itemstar.tga", TEXTURE_DIAMOND, true);
    TGA_Texture(textureArray, "texture/p.tga", TEXTURE_P, true);
    TGA_Texture(textureArray, "texture/star.tga", TEXTURE_STAR, true);
    TGA_Texture(textureArray, "texture/ring.tga", TEXTURE_RING, true);
    TGA_Texture(textureArray, "texture/long.tga", TEXTURE_LONGSHOT, true);
    TGA_Texture(textureArray, "texture/bank.tga", TEXTURE_BANKSHOT, true);
    TGA_Texture(textureArray, "texture/multi.tga", TEXTURE_MULTISHOT, true);
    TGA_Texture(textureArray, "texture/score.tga", TEXTURE_SCORE, true);
    TGA_Texture(textureArray, "texture/enemy.tga", TEXTURE_ENEMY, true);
    
    TGA_Texture(textureArray, "texture/0.tga", TEXTURE_ZERO, true);
    TGA_Texture(textureArray, "texture/1.tga", TEXTURE_ONE, true);
    TGA_Texture(textureArray, "texture/2.tga", TEXTURE_TWO, true);
    TGA_Texture(textureArray, "texture/3.tga", TEXTURE_THREE, true);
    TGA_Texture(textureArray, "texture/4.tga", TEXTURE_FOUR, true);
    TGA_Texture(textureArray, "texture/5.tga", TEXTURE_FIVE, true);
    TGA_Texture(textureArray, "texture/6.tga", TEXTURE_SIX, true);
    TGA_Texture(textureArray, "texture/7.tga", TEXTURE_SEVEN, true);
    TGA_Texture(textureArray, "texture/8.tga", TEXTURE_EIGHT, true);
    TGA_Texture(textureArray, "texture/9.tga", TEXTURE_NINE, true);
    
    
    PrepareMesh(bodymesh, "nowbody.gsm");
    PrepareMesh(turretmesh, "nowturret.gsm");
    PrepareMesh(cannonmesh, "cannon.gsm");
    PrepareMesh(itemmesh, "body.gsm");
    
    BuildDisplayLists();

    Logger::Get().Write("GraphicsTask::Started\n");
    
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
    
}



void GraphicsTask::Update()
{
    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    static int lastnumPlayers=TankHandler::GetSingleton().numPlayers;
    
    if(lastnumPlayers!=TankHandler::GetSingleton().numPlayers)
    {
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity( );
        float ratio = (float) (VideoTask::scrWidth) / (float) (VideoTask::scrHeight/2);
        gluPerspective( 45.0, ratio, 0.1, 1024.0 );
    }
    
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );
    
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    
    
    glDisable(GL_LIGHTING);
    
    if(App::GetSingleton().gameTask->gameStarted)
    {
        for(int i=0; i<TankHandler::GetSingleton().numPlayers; i++)
        {
            cams[i].SetPos(
                TankHandler::GetSingleton().players[i].x - cams[i].xzdist * (float)cos((TankHandler::GetSingleton().players[i].ry + TankHandler::GetSingleton().players[i].rty) * DTR),
                TankHandler::GetSingleton().players[i].y + cams[i].ydist,
                TankHandler::GetSingleton().players[i].z - cams[i].xzdist * (float)sin((TankHandler::GetSingleton().players[i].ry + TankHandler::GetSingleton().players[i].rty) * DTR)
            );
            
            cams[i].SetFocus(
                TankHandler::GetSingleton().players[i].x + (float)cos((TankHandler::GetSingleton().players[i].ry + TankHandler::GetSingleton().players[i].rty) * DTR),
                TankHandler::GetSingleton().players[i].y + .3,
                TankHandler::GetSingleton().players[i].z + (float)sin((TankHandler::GetSingleton().players[i].ry + TankHandler::GetSingleton().players[i].rty) * DTR)
            );
        }
    }
    
    if(TankHandler::GetSingleton().numPlayers>1)
    {
        glViewport(0, 0, VideoTask::scrWidth, VideoTask::scrHeight/2);
        //glScissor(0, 300, 800, 300);
        
        gluLookAt(cams[1].xpos(),cams[1].ypos(),cams[1].zpos(),
                  cams[1].xfocus(),cams[1].yfocus(),cams[1].zfocus(),
                  0, 1, 0);
        
        LevelHandler::GetSingleton().DrawTerrain();
        
        TankHandler::GetSingleton().DrawBullets();
        
        FXHandler::GetSingleton().Draw();
        
        LevelHandler::GetSingleton().DrawItems();
        
        TankHandler::GetSingleton().DrawTanks();
        
        if(drawHUD)
        {
            DrawHUD(TankHandler::GetSingleton().players[1]);
        }
        
        //glEnable(GL_SCISSOR_TEST);
        
        glViewport(0, 300, VideoTask::scrWidth, VideoTask::scrHeight/2);
        //glScissor(0, 0, 800, 300);
        
        glLoadIdentity();
        
        glDisable(GL_LIGHTING);
        
    }
    
    gluLookAt(cams[0].xpos(),cams[0].ypos(),cams[0].zpos(),
              cams[0].xfocus(),cams[0].yfocus(),cams[0].zfocus(),
              0, 1, 0);
    
    if(LevelHandler::GetSingleton().levelNumber==48)
    {
        DrawSky();
    }
    
    
    
    LevelHandler::GetSingleton().DrawTerrain();
    
    TankHandler::GetSingleton().DrawBullets();
    
    FXHandler::GetSingleton().Draw();
    
    LevelHandler::GetSingleton().DrawItems();
    
    TankHandler::GetSingleton().DrawTanks();
    
    
    if(drawHUD)
    {
        DrawHUD(TankHandler::GetSingleton().players[0]);
    }
    
    if(drawMenu)
    {
        DrawMenu(App::GetSingleton().gameTask->menuState);
    }
    
    if (App::GetSingleton().gameTask->debug)
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
    glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_BLACK_CUBE]);
    glCallList(cubelist1);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(400, 10, 200);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_BLACK_CUBE]);
    glCallList(cubelist1);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-400, 50, -500);
    glRotatef(TankHandler::GetSingleton().closest, 0, 0, 1);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_BLACK_CUBE]);
    glCallList(cubelist1);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-200, 40, 600);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_WHITE_CUBE]);
    glCallList(cubelist1);
    glPopMatrix();
    
    glPushMatrix();
    
    glTranslatef(-100, 40, -500);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_BLACK_CUBE]);
    glCallList(cubelist1);
    glPopMatrix();
    
    glPushMatrix();
    glColor3f(0,1,0);
    glTranslatef(300, rot/2 - 50, -500);
    glRotatef(TankHandler::GetSingleton().closest, 0, 1, 0);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_BLACK_CUBE]);
    glCallList(cubelist1);
    glPopMatrix();
    
    glPushMatrix();
    glColor3f(0,1,0);
    glTranslatef(64, 30, -500);
    glRotatef(TankHandler::GetSingleton().closest, 0, 1, 0);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_BLACK_CUBE]);
    glCallList(cubelist1);
    glPopMatrix();
    
    glPushMatrix();
    glColor3f(1,0,1);
    glTranslatef(800, 30, 90);
    glRotatef(rot, 1, 0, 1);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_BLACK_CUBE]);
    glCallList(cubelist1);
    glPopMatrix();
    
    glPushMatrix();
    glColor3f(0,1,0);
    glTranslatef(-80, 2, -900);
    glRotatef(TankHandler::GetSingleton().closest, 0, 0, 0);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_BLACK_CUBE]);
    glCallList(cubelist1);
    glPopMatrix();
    
    glPushMatrix();
    glColor3f(1,0,1);
    glTranslatef(80, -20, 450);
    glRotatef(rot, 1, 0, 0);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_BLACK_CUBE]);
    glCallList(cubelist1);
    glPopMatrix();
    
    glPushMatrix();
    glColor3f(1,1,1);
    glTranslatef(900, 20, 600);
    glRotatef(rot, 1, 0, 1);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_BLACK_CUBE]);
    glCallList(cubelist1);
    glPopMatrix();
    
    glPushMatrix();
    glColor3f(1,1,1);
    glTranslatef(-900, 30, 60);
    glRotatef(rot, 1, 0, 1);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_BLACK_CUBE]);
    glCallList(cubelist1);
    glPopMatrix();
    
    
    glPushMatrix();
    glColor3f(1,1,1);
    glTranslatef(80, 15, -600);
    glRotatef(rot, 1, 0, 1);
    glScalef(50,50,50);
    glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_BLACK_CUBE]);
    glCallList(cubelist1);
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
    glVertex3f(player.x + 32*(float)cos((player.ry+player.rty)*DTR),player.y+.1, player.z + 32*(float)sin((player.ry+player.rty)*DTR));
    
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

    textRenderer.drawString(buffer, 0, 0, 7);
    
    glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_HEART]);
    glColor4f(1.0f,0.6,0.6f,1.0f);
    
    //Armor
    //Reload
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1);
    glVertex3f(-0.55f,0.37f,(float)0);
    
    glTexCoord2f(1, 1);
    glVertex3f(-0.55f+0.03f,0.37f,(float)0);
				
    glTexCoord2f(1, 0);
    glVertex3f(-0.55f+0.03f,0.34f,(float)0);
    
    glTexCoord2f(0, 0);
    glVertex3f(-0.55f,0.34f,(float)0);
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_BANG]);
    
    glColor4f(0.6f,0.6,1.0f,1.0f);
    
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1);
    glVertex3f(-0.55f,0.32f,(float)0);
    
    glTexCoord2f(1, 1);
    glVertex3f(-0.55f+0.03f,0.32f,(float)0);
				
    glTexCoord2f(1, 0);
    glVertex3f(-0.55f+0.03f,0.29f,(float)0);
    
    glTexCoord2f(0, 0);
    glVertex3f(-0.55f,0.29f,(float)0);
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
    float amer=((float)player.energy/player.maxEnergy);
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
    float rper=((float)player.charge/player.maxCharge);
    
    if(rper>1)
        rper=1;
    
    glColor4f(0.5f,rper,1.0f,0.02f);
    
    glVertex3f(-0.51f,0.32f,0);
    glVertex3f(-0.51f+0.29f*rper,0.32f,0);
    glVertex3f(-0.51f+0.29f*rper,0.29f,0);
    glVertex3f(-0.51f,0.29f,0);
    
    if(App::GetSingleton().gameTask->debug)
    {
        //RenderText(defaultFont, 255, 255, 255, 0.0, 0.0, 0.0, "Debug Mode");
        float test=(2*(float)GlobalTimer::dT);
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
        
        glBindTexture(GL_TEXTURE_2D, textureArray[player.bonus]);
        
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
    
    
    glBindTexture(GL_TEXTURE_2D, textureArray[TankHandler::GetSingleton().hitCombo[(-1*player.id)-1]%10 ]);
    
    
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
    
    glBindTexture(GL_TEXTURE_2D, textureArray[(int)TankHandler::GetSingleton().hitCombo[(-1*player.id)-1]/10 ]);
    
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
    
    if(App::GetSingleton().gameTask->paused)
    {
        glPushMatrix();
        glLoadIdentity();
        glBindTexture(GL_TEXTURE_2D, textureArray[18]);
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
    
    if( App::GetSingleton().gameTask->versus && (!TankHandler::GetSingleton().players[0].alive || !TankHandler::GetSingleton().players[1].alive) )
    {
        glPushMatrix();
        glLoadIdentity();
        glBindTexture(GL_TEXTURE_2D, textureArray[24]);
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
        
        glBindTexture(GL_TEXTURE_2D, textureArray[TankHandler::GetSingleton().wins[(-1*player.id)-1]%10 ]);
        
        
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
        
        glBindTexture(GL_TEXTURE_2D, textureArray[(int)TankHandler::GetSingleton().wins[(-1*player.id)-1]/10 ]);
        
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

    glBindTexture(GL_TEXTURE_2D, textureArray[TankHandler::GetSingleton().tanks.size()%10]);
    
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
    
    glBindTexture(GL_TEXTURE_2D, textureArray[(int)TankHandler::GetSingleton().tanks.size()/10 ]);
    
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

    glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_X]);

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

    glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_ENEMY]);

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

    Uint8 colors = Message->format->BytesPerPixel;
    Uint8 texture_format;
    if (colors == 4) {   // alpha
        if (Message->format->Rmask == 0x000000ff)
            texture_format = GL_RGBA;
        else
            texture_format = GL_BGRA_EXT;
    }
    else {             // no alpha
        if (Message->format->Rmask == 0x000000ff)
            texture_format = GL_RGB;
        else
            texture_format = GL_BGR_EXT;
    }

    glGenTextures(1, &Texture);
    glBindTexture(GL_TEXTURE_2D, Texture); //textureArray[12]);
    //glBindTexture(GL_TEXTURE_2D, textureArray[12]);
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
    cubelist1=glGenLists(32);
    
    glNewList(cubelist1,GL_COMPILE);
    
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
    
    glEndList();
    
    cubelist2=cubelist1+1;
    
    float x=40;
    float y=1;
    float z=20;
    
    glNewList(cubelist1+1,GL_COMPILE);
    
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
    
    glEndList();
    
    bulletlist=cubelist2+1;
    
    glNewList(cubelist1+2,GL_COMPILE);
    
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
    
    
    glEndList();
    
    bodylistEx=cubelist1+3;
    
    glNewList(cubelist1+3, GL_COMPILE);
    
    bodymesh.DrawTrianglesExtruded(.01);
    
    glEndList();
    
    turretlistEx=cubelist1+4;
    
    glNewList(cubelist1+4, GL_COMPILE);
    
    turretmesh.DrawTrianglesExtruded(.01);
    
    glEndList();
    
    cannonlistEx=cubelist1+5;
    
    glNewList(cubelist1+5, GL_COMPILE);
    
    cannonmesh.DrawTrianglesExtruded(.01);
    
    glEndList();
    
    bodylist=cubelist1+6;
    
    glNewList(cubelist1+6, GL_COMPILE);
    
    bodymesh.DrawTriangles();
    
    glEndList();
    
    turretlist=cubelist1+7;
    
    glNewList(cubelist1+7, GL_COMPILE);
    
    turretmesh.DrawTriangles();
    
    glEndList();
    
    cannonlist=cubelist1+8;
    
    glNewList(cubelist1+8, GL_COMPILE);
    
    cannonmesh.DrawTriangles();
    
    glEndList();
    
    bodylistEx2=cubelist1+9;
    
    glNewList(cubelist1+9, GL_COMPILE);
    
    bodymesh.DrawEdgesExtruded(.01);
    
    glEndList();
    
    turretlistEx2=cubelist1+10;
    
    glNewList(cubelist1+10, GL_COMPILE);
    
    turretmesh.DrawEdgesExtruded(.01);
    
    glEndList();
    
    cannonlistEx2=cubelist1+11;
    
    glNewList(cubelist1+11, GL_COMPILE);
    
    cannonmesh.DrawEdgesExtruded(.01);
    
    glEndList();
    
    squarelist=cubelist1+12;
    
    glNewList(cubelist1+12, GL_COMPILE);
    
    glBegin(GL_QUADS);
    
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.0f, -0.5f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f, 0.0f, -0.5f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, 0.0f,  0.5f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, 0.0f,  0.5f);	// Bottom Right Of The Texture and Quad
    
    glEnd();
    
    glEndList();
    
    squarelist2=cubelist1+13;
    
    glNewList(cubelist1+13, GL_COMPILE);
    
    glBegin(GL_LINE_LOOP);
    
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.0f, -0.5f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f, 0.0f, -0.5f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, 0.0f,  0.5f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, 0.0f,  0.5f);	// Bottom Right Of The Texture and Quad
    
    glEnd();
    
    glEndList();
    
    itemlist=cubelist1+14;
    
    glNewList(cubelist1+14, GL_COMPILE);
    
    //glBegin(GL_LINE_LOOP);
    
    itemmesh.DrawTrianglesExtruded(.01);
    //itemmesh.DrawEdges();
    
    //glEnd();
    
    glEndList();
    
    
    alphalist[0]=glGenLists(32);
    
    glNewList(alphalist[0], GL_COMPILE);
    
    //glPushMatrix();
    
    
    glBegin(GL_LINE_LOOP); //top
    
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.50f, 0.25f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.50f, 0.25f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.50f, 0.50f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.50f, 0.50f, 0.0f);
    
    glEnd();
    
    
    glBegin(GL_LINE_LOOP); //bottom
    
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.50f, -0.25f, 0.0f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.50f, -0.25f, 0.0f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.50f,  0.00f, 0.0f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.50f,  0.00f, 0.0f);	// Bottom Right Of The Texture and Quad
    
    glEnd();
    
    
    
    
    
    glBegin(GL_LINE_LOOP); //left
    
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.50f, -0.50f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.50f,  0.50f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.25f,  0.50f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.25f, -0.50f, 0.0f);
    
    glEnd();
    
    
    glBegin(GL_LINE_LOOP); //right
    
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.50f, -0.50f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.50f,  0.50f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.25f,  0.50f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.25f, -0.50f, 0.0f);
    
    glEnd();
    
    glEndList();
    
    
    glNewList(alphalist[1], GL_COMPILE);
    
    
    glEndList();
    
    
    
    
    return;
}


