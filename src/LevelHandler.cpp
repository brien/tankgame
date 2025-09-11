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

#include <cmath>
#include "LevelHandler.h"
#include "TankHandler.h"
#include "FXHandler.h"
#include "App.h"
#include "Logger.h"
#include "rendering/RenderData.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <algorithm>
#include <fstream>

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
        // Try to load metadata first
        LoadMetadata(std::string(filePath));
        
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

    // Replace hardcoded color logic with metadata-driven approach
    if (metadata.theme.useSpecialColoring) {
        colorNumber2 = colorNumber + 2;
    } else {
        // Keep fallback for legacy levels without metadata
        if (levelNumber == 51 || levelNumber == 52 || levelNumber == 57 || 
            levelNumber == 55 || levelNumber > 70 || levelNumber == 66) {
            colorNumber2 = colorNumber + 2;
        } else {
            colorNumber2 = colorNumber;
        }
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
    
    // Use metadata colors if available, otherwise fall back to hardcoded values
    renderData.colors.defaultColor = metadata.theme.defaultColor;
    renderData.colors.blockColor = metadata.theme.blockColor;
    
    // Only log when colors change to avoid spam (using epsilon for float comparison)
    static Vector3 lastLoggedDefaultColor(-2.0f, -2.0f, -2.0f);
    static Vector3 lastLoggedBlockColor(-2.0f, -2.0f, -2.0f);
    static int lastLoggedLevel = -1;
    static bool hasLogged = false;
    
    const float epsilon = 0.001f;
    bool colorChanged = !hasLogged ||
                       lastLoggedLevel != levelNumber ||
                       std::abs(renderData.colors.defaultColor.x - lastLoggedDefaultColor.x) > epsilon || 
                       std::abs(renderData.colors.defaultColor.y - lastLoggedDefaultColor.y) > epsilon || 
                       std::abs(renderData.colors.defaultColor.z - lastLoggedDefaultColor.z) > epsilon ||
                       std::abs(renderData.colors.blockColor.x - lastLoggedBlockColor.x) > epsilon || 
                       std::abs(renderData.colors.blockColor.y - lastLoggedBlockColor.y) > epsilon || 
                       std::abs(renderData.colors.blockColor.z - lastLoggedBlockColor.z) > epsilon;
    
    if (colorChanged) {
        Logger::Get().Write("LevelHandler: Level %d - Setting colors: default(%.2f,%.2f,%.2f) block(%.2f,%.2f,%.2f)\n",
            levelNumber,
            renderData.colors.defaultColor.x, renderData.colors.defaultColor.y, renderData.colors.defaultColor.z,
            renderData.colors.blockColor.x, renderData.colors.blockColor.y, renderData.colors.blockColor.z);
        lastLoggedDefaultColor = renderData.colors.defaultColor;
        lastLoggedBlockColor = renderData.colors.blockColor;
        lastLoggedLevel = levelNumber;
        hasLogged = true;
    }
    
    // Fallback for levels without proper metadata (detect if using default values)
    if (metadata.theme.defaultColor.x == 1.0f && 
        metadata.theme.defaultColor.y == 1.0f && 
        metadata.theme.defaultColor.z == 1.0f && 
        metadata.theme.blockColor.x == 0.0f &&
        metadata.theme.blockColor.y == 1.0f &&
        metadata.theme.blockColor.z == 0.0f &&
        metadata.name == "Unnamed Level") {
        
        if (colorChanged) {
            Logger::Get().Write("LevelHandler: Level %d using fallback colors\n", levelNumber);
        }
        
        // Apply legacy hardcoded colors for specific levels
        if (levelNumber == 48) {
            renderData.colors.defaultColor = Vector3(1.0f, 1.0f, 1.0f);  // White
            renderData.colors.blockColor = Vector3(0.8f, 0.8f, 0.8f);    // Light gray
        } else if (levelNumber == 49) {
            renderData.colors.defaultColor = Vector3(0.1f, 1.0f, 0.1f);  // Green
            renderData.colors.blockColor = Vector3(0.0f, 0.7f, 0.0f);    // Dark green
        } else if (levelNumber == 50) {
            renderData.colors.defaultColor = Vector3(1.0f, 0.1f, 0.1f);  // Red
            renderData.colors.blockColor = Vector3(0.7f, 0.0f, 0.0f);    // Dark red
        } else {
            // Use a more reasonable default color instead of pure white
            renderData.colors.defaultColor = Vector3(0.7f, 0.7f, 0.7f);  // Light gray
            renderData.colors.blockColor = Vector3(0.5f, 0.5f, 0.5f);    // Dark gray
        }
        
        if (colorChanged) {
            Logger::Get().Write("LevelHandler: Level %d fallback colors applied: default(%.2f,%.2f,%.2f) block(%.2f,%.2f,%.2f)\n",
                levelNumber,
                renderData.colors.defaultColor.x, renderData.colors.defaultColor.y, renderData.colors.defaultColor.z,
                renderData.colors.blockColor.x, renderData.colors.blockColor.y, renderData.colors.blockColor.z);
        }
    }
    
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
    // Use metadata if available
    int baseCount = metadata.gameplay.baseEnemyCount;
    float multiplier = metadata.gameplay.enemyMultiplier;
    
    // Constants for enemy count calculation
    const int BASE_LEVEL = 48;      // Starting level number
    const int ENEMIES_PER_LEVEL = 3; // Additional enemies per level

    // Calculate enemy count based on level progression
    int count = static_cast<int>(baseCount + ENEMIES_PER_LEVEL * (levelNumber - BASE_LEVEL) * multiplier);
    
    // Ensure we never have fewer than the base number of enemies
    return std::max(count, baseCount);
}

// Legacy DrawTerrain_OLD() method removed - terrain rendering now handled by 
// data-driven TerrainRenderer through RenderingPipeline architecture

// ############################################################
// JSON Metadata Support
// ############################################################

std::string LevelHandler::GetMetadataPath(const std::string& levelPath) const {
    std::string metadataPath = levelPath;
    size_t dotPos = metadataPath.find_last_of('.');
    if (dotPos != std::string::npos) {
        metadataPath = metadataPath.substr(0, dotPos) + ".json";
    } else {
        metadataPath += ".json";
    }
    return metadataPath;
}

bool LevelHandler::LoadMetadata(const std::string& levelPath) {
    std::string metadataPath = GetMetadataPath(levelPath);
    
    std::ifstream file(metadataPath);
    if (!file.is_open()) {
        Logger::Get().Write("LevelHandler: No metadata file found: %s, using defaults\n", metadataPath.c_str());
        return false;
    }
    
    try {
        nlohmann::json j;
        file >> j;
        
        // Load basic metadata
        if (j.contains("name")) {
            metadata.name = j["name"].get<std::string>();
        }
        if (j.contains("author")) {
            metadata.author = j["author"].get<std::string>();
        }
        if (j.contains("difficulty")) {
            metadata.difficulty = j["difficulty"].get<int>();
        }
        
        // Load theme data
        if (j.contains("theme")) {
            auto theme = j["theme"];
            if (theme.contains("defaultColor")) {
                auto color = theme["defaultColor"];
                metadata.theme.defaultColor = Vector3(
                    color[0].get<float>(),
                    color[1].get<float>(),
                    color[2].get<float>()
                );
            }
            if (theme.contains("blockColor")) {
                auto color = theme["blockColor"];
                metadata.theme.blockColor = Vector3(
                    color[0].get<float>(),
                    color[1].get<float>(),
                    color[2].get<float>()
                );
            }
            if (theme.contains("useSpecialColoring")) {
                metadata.theme.useSpecialColoring = theme["useSpecialColoring"].get<bool>();
            }
            if (theme.contains("themeId")) {
                metadata.theme.themeId = theme["themeId"].get<int>();
            }
        }
        
        // Load gameplay data
        if (j.contains("gameplay")) {
            auto gameplay = j["gameplay"];
            if (gameplay.contains("enemyMultiplier")) {
                metadata.gameplay.enemyMultiplier = gameplay["enemyMultiplier"].get<float>();
            }
            if (gameplay.contains("itemSpawnRate")) {
                metadata.gameplay.itemSpawnRate = gameplay["itemSpawnRate"].get<float>();
            }
            if (gameplay.contains("baseEnemyCount")) {
                metadata.gameplay.baseEnemyCount = gameplay["baseEnemyCount"].get<int>();
            }
        }
        
        Logger::Get().Write("LevelHandler: Successfully loaded metadata from: %s\n", metadataPath.c_str());
        return true;
        
    } catch (const std::exception& e) {
        Logger::Get().Write("LevelHandler: Error parsing metadata file %s: %s\n", metadataPath.c_str(), e.what());
        return false;
    }
}

void LevelHandler::PrintMetadata() const {
    Logger::Get().Write("Level Metadata:\n");
    Logger::Get().Write("  Name: %s\n", metadata.name.c_str());
    Logger::Get().Write("  Author: %s\n", metadata.author.c_str());
    Logger::Get().Write("  Difficulty: %d\n", metadata.difficulty);
    Logger::Get().Write("  Default Color: (%.2f, %.2f, %.2f)\n", 
        metadata.theme.defaultColor.x, metadata.theme.defaultColor.y, metadata.theme.defaultColor.z);
    Logger::Get().Write("  Block Color: (%.2f, %.2f, %.2f)\n", 
        metadata.theme.blockColor.x, metadata.theme.blockColor.y, metadata.theme.blockColor.z);
    Logger::Get().Write("  Use Special Coloring: %s\n", metadata.theme.useSpecialColoring ? "true" : "false");
    Logger::Get().Write("  Enemy Multiplier: %.2f\n", metadata.gameplay.enemyMultiplier);
    Logger::Get().Write("  Base Enemy Count: %d\n", metadata.gameplay.baseEnemyCount);
}
