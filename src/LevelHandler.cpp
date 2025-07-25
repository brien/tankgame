#ifdef _WIN32
// If building in windows:
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
// If building on macOS:
#include <OpenGL/gl.h>
#include <mach-o/dyld.h>
#else
// If building on Linux:
#include <GL/gl.h>
#endif

#include "LevelHandler.h"
#include "TankHandler.h"
#include "FXHandler.h"
#include "App.h"
#include "rendering/RenderData.h"
#include <iostream>
#include <algorithm>

// cheap strlen function, requires a NULL TERMINATED STRING be passed.
// Returns the length of the passed string
int LevelHandler::Strlen(const char *stringy)
{
    int cp = 0;          // Incrementor
    bool cpdone = false; // boolean looper
    // Error checking
    if (stringy != NULL)
    {
        // Read entire string
        while (!cpdone)
        {
            cp++;
            // Check for null terminator
            if (stringy[cp] == '\0' || stringy[cp] == '\n')
            {
                cpdone = true;
            }
        }
    }
    return cp;
}

LevelHandler::LevelHandler() : items(), drawFloor(true), drawWalls(false), drawTop(false), start{0}, enemy{0}, fileName{""}, sizeX(128), sizeZ(128), levelNumber(48), colorNumber(0), colorNumber2(0)
{
}

void LevelHandler::Init()
{
    levelNumber = 48;
    sizeX = MAX_SIZE_X;
    sizeZ = MAX_SIZE_Z;
    colorNumber = 0;
    drawFloor = true;
    drawWalls = false;
    drawTop = false;
}

bool LevelHandler::Load(const char filePath[])
{

    for (int q = 0; q < 128; q++)
    {
        for (int w = 0; w < 128; w++)
        {
            f[q][w] = 0;
        }
    }

    Flatten(1);

    for (int e = 0; e < 16; e++)
    {
        enemy[e][0] = 64;
        enemy[e][1] = 64;
    }

    if (filePath != NULL)
    {
        FILE *filein;
        int errnum;
        filein = fopen(filePath, "rt");

        // Error checking
        if (!filein)
        {
            Logger::Get().Write("ERROR: LevelHandler: failed to open file: %s  \n", filePath);
            std::cerr << "ERROR: LevelHandler: failed to open file: " << filePath << std::endl;

            errnum = errno;
            fprintf(stderr, "Value of errno: %d\n", errno);
            perror("Error printed by perror");
            fprintf(stderr, "Error opening file: %s\n", strerror(errnum));

            Logger::Get().Write("ERROR: LevelHandler: errnum: %d", strerror(errnum));

            return false;
        }
        else
        {
            // fileName=new char[Strlen(filePath)+1];
            // fileName[Strlen(filePath)]='\n';

            for (int k = Strlen(filePath); k > -1; k--)
            {
                fileName[k] = filePath[k];
            }

            // These two lines get the map size
            // The last two characters in the file's name are the map's size.
            // Ex:
            // level@0.txt
            // That means that sizeX=64 and sizeZ=48
            // This is not yet implimented because we're using static arrays

            // sizeX=2*filePath[Strlen(filePath)-6];

            // sizeZ=2*filePath[Strlen(filePath)-5];

            sizeX = 128;
            sizeZ = 128;

            /*

             t = new int*[sizeX];


             //Recall that ap[i] is a pointer to an array of int's
             t[0]=new int[sizeX*sizeZ];
             // This "connects" the array of pointers to the arrays of ints
             for (int m=1;m<sizeX;m++)
             {
             t[m]=t[m-1]+sizeZ;
             }*/

            int cdepth = sizeZ;
            char *oneline = new char[cdepth]; // Storage per line

            for (int i = 0; i < sizeX; i++)
            {
                // Get next line
                fgets(oneline, cdepth, filein);
                for (int j = 0; j < sizeZ; j++)
                {
                    t[i][j] = oneline[j];

                    if (t[i][j] == 33)
                    {
                        start[0] = i;
                        start[1] = j;
                        t[i][j] = t[i - 1][j];
                    }
                    else if (t[i][j] >= 48 && t[i][j] < 58)
                    {
                        enemy[t[i][j] - 48][0] = i;
                        enemy[t[i][j] - 48][1] = j;
                        t[i][j] = t[i - 1][j];
                    }
                    else
                    {
                        t[i][j] = oneline[j] - 100;

                        if (t[i][j] == -1)
                        {
                            if (App::GetSingleton().gameTask->IsVersusMode())
                            {
                                t[i][j] = 10;
                            }
                            else
                            {
                                t[i][j] = -20;
                            }
                        }

                        if (t[i][j] > 26)
                            t[i][j] = 0;
                        if (i == 0 || i == (sizeX - 1) || j == 0 || j == (sizeZ - 1))
                            t[i][j] = 26;
                    }
                }
                fgets(oneline, cdepth, filein);
            }

            delete[] oneline;

            Logger::Get().Write("LevelHandler: finished loading file: %s  \n", filePath);
            fclose(filein);
            return true;
        }
    }

    levelNumber = fileName[5];

    return false;
}

void LevelHandler::NextLevel(bool forb)
{
    FXHandler::GetSingleton().ClearFX();
    BulletHandler::GetSingleton().Clear();

    items.clear();

    levelNumber = fileName[12];

    if (forb)
    {
        levelNumber++;
        colorNumber++;
    }

    if (colorNumber >= 15)
    {
        colorNumber = 0;
    }

    if (levelNumber == 58)
    {
        if (forb)
        {
            levelNumber = 65;
        }
    }

    if (levelNumber < 76)
    {
        fileName[12] = levelNumber;
    }
    else
    {
        fileName[12] = levelNumber - 25;
    }

    Load(fileName);

    if (levelNumber == 51 || levelNumber == 52 || levelNumber == 57 || levelNumber == 55 || levelNumber > 70 || levelNumber == 66)
    {
        colorNumber2 = colorNumber + 2;
    }
    else
    {
        colorNumber2 = colorNumber;
    }

    TankHandler::GetSingleton().Init();

    return;
}

void LevelHandler::Flatten(int height)
{
    int i = 0;
    while (i < sizeX)
    {
        int j = 0;
        while (j < sizeZ)
        {
            t[i][j] = height;
            j++;
        }
        i++;
    }
}

bool LevelHandler::FloatCollision(float x, float y, float z)
{

    bool ret;

    if (x >= sizeX || z >= sizeZ)
    {
        return (true);
    }

    if (y < f[(int)x][(int)z] && y > (f[(int)x][(int)z] - 1) && f[(int)x][(int)z] > 0)
    {
        ret = true;
    }
    else
    {
        ret = false;
    }

    return (ret);
}

bool LevelHandler::FallCollision(float x, float y, float z)
{

    bool ret;

    if (x >= sizeX || z >= sizeZ)
    {
        return (true);
    }

    if (!(y > t[(int)x][(int)z]))
    {
        ret = true;
    }
    else
    {
        ret = false;
    }

    return (ret);
}

void LevelHandler::AddItem(float x, float y, float z, TankType type)
{
    if (x < sizeX || z < sizeZ)
    {
        Item temp(x, t[(int)x][(int)z] + .2, z, type);
        items.push_back(temp);
    }
}

void LevelHandler::UpdateItems()
{
    for (auto& item : items) {
        if (item.alive) {
            item.Update();
        }
    }
}

void LevelHandler::ItemCollision()
{
    for (vector<Item>::iterator j = items.begin(); j != items.end(); j++)
    {
        for (int i = 0; i < TankHandler::GetSingleton().numPlayers; i++)
        {
            if (TankHandler::GetSingleton().players[i].PointCollision(j->x, j->y, j->z))
            {
                TankHandler::GetSingleton().players[i].SetType(j->type, TankHandler::GetSingleton().players[i].type1);

                if (j->type == TankHandler::GetSingleton().players[i].type1)
                    TankHandler::GetSingleton().players[i].energy += TankHandler::GetSingleton().players[i].maxEnergy;
                else
                    TankHandler::GetSingleton().players[i].energy += TankHandler::GetSingleton().players[i].maxEnergy / 2;

                if (TankHandler::GetSingleton().players[i].energy > TankHandler::GetSingleton().players[i].maxEnergy)
                    TankHandler::GetSingleton().players[i].energy = TankHandler::GetSingleton().players[i].maxEnergy;

                j->alive = false;

                FXHandler::GetSingleton().CreateFX(FxType::TYPE_THREE, j->x, j->y, j->z, 90, 0, 90, j->r, j->g, j->b, 1);

                App::GetSingleton().soundTask->PlayChannel(3);
                return;
            }
        }
    }
}

bool LevelHandler::PointCollision(float x, float y, float z)
{
    bool ret;

    if (x < 0 || z < 0 || x >= sizeX || z >= sizeZ)
    {
        return (true);
    }
    else
    {
        if ((int)y < t[(int)x][(int)z])
        {
            ret = true;
        }
        else
        {
            ret = false;
        }

        if (y < f[(int)x][(int)z] && y >= (f[(int)x][(int)z] - 1) && f[(int)x][(int)z] > 0)
        {
            ret = true;
        }
    }

    return (ret);
}

int LevelHandler::GetTerrainHeight(int x, int z)
{
    if (x < 0 || x > 127 || z < 0 || z > 127)
    {
        return 666;
    }
    else
    {
        return t[x][z];
    }
}

int LevelHandler::GetFloatHeight(int x, int z)
{
    if (x < 0 || x > 127 || z < 0 || z > 127)
    {
        return 666;
    }
    else
    {
        return f[x][z];
    }
}

bool LevelHandler::SetTerrainHeight(int x, int z, int height)
{
    if (x < 0 || x > 127 || z < 0 || z > 127)
    {
        return false;
    }
    else
    {
        t[x][z] = height;
        return true;
    }
}

bool LevelHandler::HandlePointCollision(float &x, float &y, float &z, float &vx, float &vz)
{
    x -= vx;
    z -= vz;

    int Xint = (int)x;
    int Zint = (int)z;

    x += vx;
    z += vz;

    int Xor = (int)x;
    int Zor = (int)z;

    if ((int)x != Xint && (int)z != Zint)
    {
        if ((vz < 0 && LevelHandler::PointCollision(x, y, z + 1)) ||
            ((vz > 0 && LevelHandler::PointCollision(x, y, z - 1)) && (vx < 0 && LevelHandler::PointCollision(x + 1, y, z))) ||
            (vx > 0 && LevelHandler::PointCollision(x - 1, y, z)))
        {
            x -= vx;
            vx = 0;
            z -= vz;
            vz = 0;
        }
        else
        {

            if (vx > 0 && LevelHandler::PointCollision(x - 1, y, z))
            {
                z -= vz;
                vz = 0;
            }
            // else
            if (vx < 0 && LevelHandler::PointCollision(x + 1, y, z))
            {
                z -= vz;
                vz = 0;
            }
            // else
            if (vz > 0 && LevelHandler::PointCollision(x, y, z - 1))
            {
                x -= vx;
                vx = 0;
            }
            // else
            if (vz < 0 && LevelHandler::PointCollision(x, y, z + 1))
            {
                x -= vx;
                vx = 0;
            }
        }
    }
    else
    {

        if ((int)x != Xint)
        {
            x -= vx;
            if (vx > 0)
            {
                x = Xor - .001;
            }
            else
            {
                x = Xint + .001;
            }

            vx = 0;
        }
        if ((int)z != Zint)
        {
            z -= vz;
            if (vz > 0)
            {
                z = Zor - .001;
            }
            else
            {
                z = Zint + .001;
            }
            vz = 0;
        }
    }
    return (1);
}

void LevelHandler::GenerateTerrain()
{
    for (int i = 0; i < sizeX; i++)
    {
        for (int j = 0; j < sizeZ; j++)
        {
            t[i][j] = 2;
        }
    }
}


void LevelHandler::populateTerrainRenderData(TerrainRenderData& renderData) const {
    renderData.levelNumber = levelNumber;
    renderData.colorNumber = colorNumber;
    renderData.colorNumber2 = colorNumber2;
    renderData.sizeX = sizeX;
    renderData.sizeZ = sizeZ;
    renderData.drawFloor = drawFloor;
    renderData.drawWalls = drawWalls;
    renderData.drawTop = drawTop;
    
    // Set terrain colors based on level number (moved from DrawTerrain_OLD OpenGL calls)
    if (levelNumber == 48) {
        renderData.colors.defaultColor = Vector3(1.0f, 1.0f, 1.0f);  // White
    } else if (levelNumber == 49) {
        renderData.colors.defaultColor = Vector3(0.1f, 1.0f, 0.1f);  // Green
    } else if (levelNumber == 50) {
        renderData.colors.defaultColor = Vector3(1.0f, 0.1f, 0.1f);  // Red
    } else {
        renderData.colors.defaultColor = Vector3(1.0f, 1.0f, 1.0f);  // Default white
    }
    
    // Set block color for terrain rendering (moved from line 740)
    renderData.colors.blockColor = Vector3(0.0f, 1.0f, 0.0f);  // Green
    
    // Copy height map data
    for (int x = 0; x < TerrainRenderData::MAX_SIZE_X && x < sizeX; x++) {
        for (int z = 0; z < TerrainRenderData::MAX_SIZE_Z && z < sizeZ; z++) {
            renderData.heightMap[x][z] = t[x][z];
            renderData.floatMap[x][z] = f[x][z];
        }
    }
}

// ############################################################

// data-driven TerrainRenderer through RenderingPipeline architecture

int LevelHandler::GetEnemyCountForLevel(int levelNumber) const
{
    // Constants for enemy count calculation
    const int BASE_LEVEL = 48;      // Starting level number
    const int BASE_ENEMIES = 5;     // Minimum number of enemies
    const int ENEMIES_PER_LEVEL = 3; // Additional enemies per level

    // Calculate enemy count based on level progression
    int count = BASE_ENEMIES + ENEMIES_PER_LEVEL * (levelNumber - BASE_LEVEL);
    
    // Ensure we never have fewer than the base number of enemies
    return std::max(count, BASE_ENEMIES);
}

// Legacy DrawTerrain_OLD() method removed - terrain rendering now handled by 
// data-driven TerrainRenderer through RenderingPipeline architecture
