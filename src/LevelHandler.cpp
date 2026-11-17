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
#include "GameWorld.h"
#include "TankHandler.h"
#include "PlayerManager.h"
#include "App.h"
#include "Logger.h"
#include "events/Events.h"
#include "rendering/RenderData.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <limits>
#include <stdexcept>

void LevelHandler::CreateFX(FxType type, float x, float y, float z, float rx, float ry, float rz, float r, float g, float b, float a)
{
    gameWorld->CreateFX(type, x, y, z, rx, ry, rz, r, g, b, a);
}

void LevelHandler::CreateItemCollectionFX(float x, float y, float z, const Color& color)
{
    // Create spinning collection effect with item's color
    CreateFX(FxType::TYPE_THREE, x, y, z, 90, 0, 90, color.r, color.g, color.b, 1);
}

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

LevelHandler::LevelHandler() : drawFloor(true), drawWalls(false), drawTop(false), start{0}, enemy{0}, fileName{""}, sizeX(128), sizeZ(128), levelNumber(48), colorNumber(0), colorNumber2(0)
{
}

void LevelHandler::Init()
{
    metadata = LevelMetadata{};
    levelChangePending = false;
    advanceLevel = false;
    levelNumber = 48;
    sizeX = MAX_SIZE_X;
    sizeZ = MAX_SIZE_Z;
    colorNumber = 0;
    colorNumber2 = 0;
    drawFloor = true;
    drawWalls = false;
    drawTop = false;
}

bool LevelHandler::Load(const char filePath[])
{
    // Every load starts independently, including empty worlds and missing maps.
    metadata = LevelMetadata{};

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

            errnum = errno;
            Logger::Get().Write("Value of errno: %d\n", errno);
            perror("Error printed by perror");
            Logger::Get().Write("Error opening file: %s\n", strerror(errnum));

            Logger::Get().Write("ERROR: LevelHandler: errnum: %d", strerror(errnum));

            return false;
        }
        else
        {
            fileName = filePath;

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

    // Load(nullptr) is used to initialize an empty test world.

    return false;
}

void LevelHandler::RequestNextLevel(bool forward)
{
    // Coalesce requests from both players; a death/retry takes priority.
    advanceLevel = levelChangePending ? advanceLevel && forward : forward;
    levelChangePending = true;
}

void LevelHandler::ProcessPendingLevelChange()
{
    if (!levelChangePending) return;
    levelChangePending = false;
    NextLevel(advanceLevel);
}

void LevelHandler::NextLevel(bool forb)
{
    Logger::Get().Write("\n========== LevelHandler::NextLevel(%s) called ==========\n", forb ? "true" : "false");

    // Bullet updates may have queued collisions before a respawn reloads the
    // level in the same frame. Discard old-level events before their entities
    // are destroyed, just as GameTask::SetUpGame does for a new game.
    Events::Clear();
    
    // Clear GameWorld entities (bullets, effects, items) 
    Logger::Get().Write("LevelHandler: Clearing GameWorld...\n");
    if (players) players->ReleaseTanks();
    gameWorld->Clear();
    Logger::Get().Write("LevelHandler: GameWorld cleared\n");

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

    const std::string nextFile = fileName;
    Load(nextFile.c_str());

    // Replace hardcoded color logic with metadata-driven approach
    if (metadata.theme.useSpecialColoring)
    {
        colorNumber2 = colorNumber + 2;
    }
    else
    {
        // Keep fallback for legacy levels without metadata
        if (levelNumber == 51 || levelNumber == 52 || levelNumber == 57 ||
            levelNumber == 55 || levelNumber > 70 || levelNumber == 66)
        {
            colorNumber2 = colorNumber + 2;
        }
        else
        {
            colorNumber2 = colorNumber;
        }
    }

    TankHandler::GetSingleton().Init();
    if (players) players->SpawnPlayerTanks();

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
    gameWorld->CreateItem(x, y, z, type);
}

void LevelHandler::UpdateItems()
{
    // All items now managed by GameWorld - updates happen in GameWorld::Update()
    // This method kept for API compatibility but is now a no-op
}

void LevelHandler::ItemCollision()
{
    // Share the player-owned pickup path, including rewards and difficulty stats.
    if (!gameWorld) return;
    auto* manager = App::GetSingleton().gameTask->GetPlayerManager();
    for (int i = 0; i < manager->GetNumPlayers(); ++i) {
        if (auto* player = manager->GetPlayer(i)) player->CheckItemCollisions();
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

void LevelHandler::populateTerrainRenderData(TerrainRenderData &renderData) const
{
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

    if (colorChanged)
    {
        int logicalLevel = ConvertInternalToLogicalLevel(levelNumber);
        Logger::Get().Write("LevelHandler: Level %d (internal %d) - Setting colors: default(%.2f,%.2f,%.2f) block(%.2f,%.2f,%.2f)\n",
                            logicalLevel, levelNumber,
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
        metadata.name == "Unnamed Level")
    {

        if (colorChanged)
        {
            int logicalLevel = ConvertInternalToLogicalLevel(levelNumber);
            Logger::Get().Write("LevelHandler: Level %d (internal %d) using fallback colors\n", logicalLevel, levelNumber);
        }

        // Apply legacy hardcoded colors for specific levels
        if (levelNumber == 48)
        {
            renderData.colors.defaultColor = Vector3(1.0f, 1.0f, 1.0f); // White
            renderData.colors.blockColor = Vector3(0.8f, 0.8f, 0.8f);   // Light gray
        }
        else if (levelNumber == 49)
        {
            renderData.colors.defaultColor = Vector3(0.1f, 1.0f, 0.1f); // Green
            renderData.colors.blockColor = Vector3(0.0f, 0.7f, 0.0f);   // Dark green
        }
        else if (levelNumber == 50)
        {
            renderData.colors.defaultColor = Vector3(1.0f, 0.1f, 0.1f); // Red
            renderData.colors.blockColor = Vector3(0.7f, 0.0f, 0.0f);   // Dark red
        }
        else
        {
            // Use a more reasonable default color instead of pure white
            renderData.colors.defaultColor = Vector3(0.7f, 0.7f, 0.7f); // Light gray
            renderData.colors.blockColor = Vector3(0.5f, 0.5f, 0.5f);   // Dark gray
        }

        if (colorChanged)
        {
            int logicalLevel = ConvertInternalToLogicalLevel(levelNumber);
            Logger::Get().Write("LevelHandler: Level %d (internal %d) fallback colors applied: default(%.2f,%.2f,%.2f) block(%.2f,%.2f,%.2f)\n",
                                logicalLevel, levelNumber,
                                renderData.colors.defaultColor.x, renderData.colors.defaultColor.y, renderData.colors.defaultColor.z,
                                renderData.colors.blockColor.x, renderData.colors.blockColor.y, renderData.colors.blockColor.z);
        }
    }

    // Copy height map data
    for (int x = 0; x < TerrainRenderData::MAX_SIZE_X && x < sizeX; x++)
    {
        for (int z = 0; z < TerrainRenderData::MAX_SIZE_Z && z < sizeZ; z++)
        {
            renderData.heightMap[x][z] = t[x][z];
            renderData.floatMap[x][z] = f[x][z];
        }
    }
}

// ############################################################

// data-driven TerrainRenderer through RenderingPipeline architecture

int LevelHandler::GetEnemyCountForLevel(int levelNumber) const
{
    // Preserve ASCII-based progression (including the 9 -> A gap), but avoid
    // integer arithmetic overflow and out-of-range floating-point conversion.
    const int baseCount = metadata.gameplay.baseEnemyCount;
    const float progression = static_cast<float>(3.0 * (static_cast<double>(levelNumber) - 48.0));
    // Keep the original float rounding for ordinary tuning values.
    const float count = baseCount + progression * metadata.gameplay.enemyMultiplier;
    if (static_cast<double>(count) <= baseCount) return baseCount;
    if (static_cast<double>(count) >= std::numeric_limits<int>::max()) return std::numeric_limits<int>::max();
    return static_cast<int>(count);

}

// Legacy DrawTerrain_OLD() method removed - terrain rendering now handled by
// data-driven TerrainRenderer through RenderingPipeline architecture

// ############################################################
// JSON Metadata Support
// ############################################################

std::string LevelHandler::GetMetadataPath(const std::string &levelPath) const
{
    std::string metadataPath = levelPath;
    size_t dotPos = metadataPath.find_last_of('.');
    const size_t separator = metadataPath.find_last_of("/\\");
    if (dotPos != std::string::npos && (separator == std::string::npos || dotPos > separator))
    {
        metadataPath = metadataPath.substr(0, dotPos) + ".json";
    }
    else
    {
        metadataPath += ".json";
    }
    return metadataPath;
}

namespace {
void RequireMetadata(bool condition, const std::string& field) {
    if (!condition) throw std::invalid_argument("invalid level metadata field: " + field);
}

int MetadataInteger(const nlohmann::json& object, const char* key, int fallback,
                    int minimum, const std::string& path) {
    if (!object.contains(key)) return fallback;
    const auto& value = object.at(key);
    RequireMetadata(value.is_number_integer(), path);
    // All int values are exact in double; reject large unsigned JSON numbers
    // before converting to the gameplay integer representation.
    const double number = value.get<double>();
    RequireMetadata(number >= minimum && number <= std::numeric_limits<int>::max(), path);
    return static_cast<int>(number);
}

Vector3 MetadataColor(const nlohmann::json& color, const std::string& path) {
    RequireMetadata(color.is_array() && color.size() == 3, path);
    float channels[3];
    for (size_t i = 0; i < 3; ++i) {
        RequireMetadata(color.at(i).is_number(), path);
        const double value = color.at(i).get<double>();
        RequireMetadata(std::isfinite(value) && value >= 0 && value <= 1, path);
        channels[i] = static_cast<float>(value);
    }
    return Vector3(channels[0], channels[1], channels[2]);
}

LevelMetadata ParseLevelMetadata(const nlohmann::json& json) {
    RequireMetadata(json.is_object(), "root");
    LevelMetadata candidate;
    if (json.contains("name")) {
        RequireMetadata(json.at("name").is_string(), "name");
        candidate.name = json.at("name").get<std::string>();
    }
    if (json.contains("author")) {
        RequireMetadata(json.at("author").is_string(), "author");
        candidate.author = json.at("author").get<std::string>();
    }
    candidate.difficulty = MetadataInteger(json, "difficulty", candidate.difficulty, 0, "difficulty");
    if (json.contains("theme")) {
        const auto& theme = json.at("theme");
        RequireMetadata(theme.is_object(), "theme");
        if (theme.contains("defaultColor"))
            candidate.theme.defaultColor = MetadataColor(theme.at("defaultColor"), "theme.defaultColor");
        if (theme.contains("blockColor"))
            candidate.theme.blockColor = MetadataColor(theme.at("blockColor"), "theme.blockColor");
        if (theme.contains("useSpecialColoring")) {
            RequireMetadata(theme.at("useSpecialColoring").is_boolean(), "theme.useSpecialColoring");
            candidate.theme.useSpecialColoring = theme.at("useSpecialColoring").get<bool>();
        }
        candidate.theme.themeId = MetadataInteger(theme, "themeId", candidate.theme.themeId,
                                                  -1, "theme.themeId");
    }
    if (json.contains("gameplay")) {
        const auto& gameplay = json.at("gameplay");
        RequireMetadata(gameplay.is_object(), "gameplay");
        if (gameplay.contains("enemyMultiplier")) {
            const auto& value = gameplay.at("enemyMultiplier");
            RequireMetadata(value.is_number(), "gameplay.enemyMultiplier");
            const double multiplier = value.get<double>();
            RequireMetadata(std::isfinite(multiplier) && multiplier >= 0 &&
                multiplier <= std::numeric_limits<float>::max(), "gameplay.enemyMultiplier");
            candidate.gameplay.enemyMultiplier = static_cast<float>(multiplier);
        }
        candidate.gameplay.baseEnemyCount = MetadataInteger(gameplay, "baseEnemyCount",
            candidate.gameplay.baseEnemyCount, 0, "gameplay.baseEnemyCount");
        // itemSpawnRate was never consumed by gameplay. Ignore legacy copies,
        // like other unknown keys, instead of implying an item-spawning control.
    }
    return candidate;
}
}

bool LevelHandler::LoadMetadata(const std::string &levelPath)
{
    const std::string metadataPath = GetMetadataPath(levelPath);
    std::ifstream file(metadataPath);
    if (!file.is_open()) {
        Logger::Get().Write("LevelHandler: No metadata file found: %s, using defaults\n", metadataPath.c_str());
        return false;
    }
    try {
        // Commit only a complete, validated candidate; errors leave this load's
        // fresh defaults intact. parse() also rejects trailing JSON content.
        auto candidate = ParseLevelMetadata(nlohmann::json::parse(file));
        metadata = std::move(candidate);
        Logger::Get().Write("LevelHandler: Successfully loaded metadata from: %s\n", metadataPath.c_str());
        return true;
    } catch (const std::exception& error) {
        Logger::Get().Write("LevelHandler: Invalid metadata file %s: %s; using defaults\n",
                            metadataPath.c_str(), error.what());
        return false;
    }
}

void LevelHandler::PrintMetadata() const
{
    Logger::Get().Write("Level Metadata:\n");
    Logger::Get().Write("  Name: %s\n", metadata.name.c_str());
    Logger::Get().Write("  Author: %s\n", metadata.author.c_str());
    Logger::Get().Write("  Descriptive difficulty: %d\n", metadata.difficulty);
    Logger::Get().Write("  Default Color: (%.2f, %.2f, %.2f)\n",
                        metadata.theme.defaultColor.x, metadata.theme.defaultColor.y, metadata.theme.defaultColor.z);
    Logger::Get().Write("  Block Color: (%.2f, %.2f, %.2f)\n",
                        metadata.theme.blockColor.x, metadata.theme.blockColor.y, metadata.theme.blockColor.z);
    Logger::Get().Write("  Use Special Coloring: %s\n", metadata.theme.useSpecialColoring ? "true" : "false");
    Logger::Get().Write("  Enemy Multiplier: %.2f\n", metadata.gameplay.enemyMultiplier);
    Logger::Get().Write("  Base Enemy Count: %d\n", metadata.gameplay.baseEnemyCount);
}

// Helper functions for level number conversion
int LevelHandler::GetLogicalLevelNumber() const
{
    return ConvertInternalToLogicalLevel(levelNumber);
}

void LevelHandler::SetLogicalLevelNumber(int logicalLevel)
{
    levelNumber = ConvertLogicalToInternalLevel(logicalLevel);
}

int LevelHandler::ConvertLogicalToInternalLevel(int logicalLevel)
{
    // Convert 0-based logical level numbers to internal ASCII-based system
    // Logical 0 → Internal 48 (ASCII '0')
    // Logical 1 → Internal 49 (ASCII '1')
    // ...
    // Logical 9 → Internal 57 (ASCII '9')
    // Logical 10 → Internal 65 (ASCII 'A')
    // Logical 11 → Internal 66 (ASCII 'B')
    // etc.

    if (logicalLevel <= 9)
    {
        return 48 + logicalLevel; // ASCII '0' through '9'
    }
    else
    {
        return 65 + (logicalLevel - 10); // ASCII 'A' through 'Z'
    }
}

int LevelHandler::ConvertInternalToLogicalLevel(int internalLevel)
{
    // Convert internal ASCII-based system back to 0-based logical levels
    if (internalLevel >= 48 && internalLevel <= 57)
    {
        return internalLevel - 48; // ASCII '0' through '9' → 0-9
    }
    else if (internalLevel >= 65 && internalLevel <= 90)
    {
        return (internalLevel - 65) + 10; // ASCII 'A' through 'Z' → 10-35
    }
    else
    {
        return 0; // Fallback to level 0 for invalid values
    }
}

