//
//  GraphicsTask.h
//  tankgame
//
//  Created by Brien Smith Martínez on 27/12/15.
//  Copyright © 2015 Brien Smith Martínez. All rights reserved.
//

#ifndef GraphicsTask_h
#define GraphicsTask_h

#include "BaseTask.h"
#include "igtl_qmesh.h"
#include "Tank.h"

class Camera
{
public:
    Camera();
    ~Camera();
    
    //void RemoveFocus();
    float xpos;
    float ypos;
    float zpos;
    
    float xfocus;
    float yfocus;
    float zfocus;
    
    float xzdist;
    float ydist;
};

#define TGA_RGB		2
#define TGA_A		3
#define TGA_RLE		10



struct tImageTGA
{
    int channels;
    int size_x;
    int size_y;
    unsigned char *data;
};

class GraphicsTask : public BaseTask
{
public:
    GraphicsTask();
    virtual ~GraphicsTask();
    
    unsigned int textureArray[32];
    
    void TGA_Texture(unsigned int textureArray[], char* strFileName, int ID, bool wrap);
    tImageTGA *Load_TGA(const char *strfilename);
    
    void BuildDisplayLists();
    
    Camera cams[4];
    //Camera cam2;
    
    int alphalist[26];
    
    int cubelist1;
    int cubelist2;
    int bulletlist;
    int bodylist;
    int turretlist;
    int cannonlist;
    
    int itemlist;
    
    int squarelist;
    int squarelist2;
    
    int bodylistEx;
    int turretlistEx;
    int cannonlistEx;
    
    int bodylistEx2;
    int turretlistEx2;
    int cannonlistEx2;
    
    void DrawHUD(Tank& player);
    void DrawMenu(int option);
    void DrawSky();
    
    bool Start();
    void Update();
    void Stop();
    
    bool drawHUD;
    bool drawMenu;
    
    igtl_QGLMesh bodymesh;
    igtl_QGLMesh turretmesh;
    igtl_QGLMesh cannonmesh;
    
    igtl_QGLMesh itemmesh;
    
};
#endif /* GraphicsTask_h */
