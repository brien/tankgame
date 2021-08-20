//
//  GraphicsTask.h
//  tankgame
//
//

#ifndef GraphicsTask_h
#define GraphicsTask_h

#ifdef _WIN32
// If building in windows:
#include <windows.h>
#include <GL/gl.h>
#else
//if not:
#include <OpenGL/gl.h>
#endif

#include <SDL2_ttf/SDL_ttf.h>

#include "ITask.h"
#include "igtl_qmesh.h"
#include "Tank.h"
#include "Camera.h"


#define TGA_RGB		2
#define TGA_A		3
#define TGA_RLE		10

enum TextureNames
{
    TEXTURE_ZERO,
    TEXTURE_ONE,
    TEXTURE_TWO,
    TEXTURE_THREE,
    TEXTURE_FOUR,
    TEXTURE_FIVE,
    TEXTURE_SIX,
    TEXTURE_SEVEN,
    TEXTURE_EIGHT,
    TEXTURE_NINE,
    TEXTURE_WHITE_CUBE,
    TEXTURE_BLACK_CUBE,
    TEXTURE_EXIT,
    TEXTURE_BANG,
    TEXTURE_X,
    TEXTURE_CHECKER,
    TEXTURE_HEART,
    TEXTURE_DIAMOND,
    TEXTURE_P,
    TEXTURE_STAR,
    TEXTURE_RING,
    TEXTURE_LONGSHOT,
    TEXTURE_BANKSHOT,
    TEXTURE_MULTISHOT,
    TEXTURE_SCORE,
    
    TEXTURE_NAMES_COUNT
};


struct tImageTGA
{
    int channels;
    int size_x;
    int size_y;
    unsigned char *data;
};

class GraphicsTask : public ITask
{
public:
    GraphicsTask();
    virtual ~GraphicsTask();
    
    unsigned int textureArray[32];
    TTF_Font *defaultFont;
    
    void TGA_Texture(unsigned int textureArray[], const char* strFileName, int ID, bool wrap);
    tImageTGA *Load_TGA(const char *strfilename);
    
    void BuildDisplayLists();
    
    Camera cams[4];
    
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
    void RenderText(const TTF_Font* Font, const unsigned char& R, const unsigned char& G, const unsigned char& B, const double& X, const double& Y, const double& Z, const char* Text);
    
    bool Start();
    void Update();
    void Stop();
    
    bool drawHUD;
    bool drawMenu;
private:
    igtl_QGLMesh bodymesh;
    igtl_QGLMesh turretmesh;
    igtl_QGLMesh cannonmesh;
    igtl_QGLMesh itemmesh;

    void FixMesh(igtl_QGLMesh& mesh);
    void PrepareMesh(igtl_QGLMesh& mesh, const char* fileName);
};
#endif /* GraphicsTask_h */
