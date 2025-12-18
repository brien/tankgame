//
//  LevelHandler.h
//  tankgame
//
//

#pragma once

#include <vector>
#include <string>
using namespace std;
#include "Item.h"
#include "Singleton.h"
#include "rendering/RenderData.h"

// Forward declarations
struct TerrainRenderData;
class GameWorld;
enum class FxType;

/**
 * Level metadata structure for JSON-based level configuration
 */
struct LevelMetadata {
    std::string name = "Unnamed Level";
    std::string author = "Unknown";
    int difficulty = 1;
    
    struct Theme {
        Vector3 defaultColor = Vector3(1.0f, 1.0f, 1.0f);
        Vector3 blockColor = Vector3(0.0f, 1.0f, 0.0f);
        bool useSpecialColoring = false;
        int themeId = 0;
    } theme;
    
    struct Gameplay {
        float enemyMultiplier = 1.0f;
        float itemSpawnRate = 1.0f;
        int baseEnemyCount = 5;
    } gameplay;
};

class LevelHandler : public Singleton<LevelHandler>
{
public:
    LevelHandler();
    ~LevelHandler() = default;

    void Init();

    bool Load(const char filepath[]);

    bool drawFloor;
    bool drawWalls;
    bool drawTop;

    int start[2];
    int enemy[16][2];

    char fileName[18];
    int sizeX;
    int sizeZ;

    void Flatten(int height);
    bool PointCollision(float x, float y, float z);
    void ItemCollision();
    void UpdateItems();  // Updates item states (rotation animations)
    void AddItem(float x, float y, float z, TankType type);

    // Interface to GameWorld
    void SetGameWorld(GameWorld* world) { gameWorld = world; }
    
    // Descriptive FX helper methods (encapsulate effect creation logic)
    void CreateItemCollectionFX(float x, float y, float z, const Color& color);
    
    // Low-level FX creation (used by helper methods)
    void CreateFX(FxType type, float x, float y, float z, float rx, float ry, float rz, float r, float g, float b, float a);

    bool HandlePointCollision(float &x, float &y, float &z, float &vx, float &vz);
    bool FallCollision(float x, float y, float z);
    bool FloatCollision(float x, float y, float z);
    int GetTerrainHeight(int x, int z);
    int GetFloatHeight(int x, int z);
    bool SetTerrainHeight(int x, int z, int height);
    void GenerateTerrain();

    // Rendering data extraction for new rendering pipeline
    void populateTerrainRenderData(struct TerrainRenderData& renderData) const;

    // Legacy rendering methods removed - terrain rendering now handled by TerrainRenderer
    // void DrawTerrain_OLD(); // REMOVED - replaced by data-driven TerrainRenderer

    int Strlen(const char *stringy);

    void NextLevel(bool forb);

    // Enemy count calculation for level scaling
    int GetEnemyCountForLevel(int levelNumber) const;

    int levelNumber;
    int colorNumber;
    int colorNumber2;
    
    // Helper functions for level number conversion
    int GetLogicalLevelNumber() const;
    void SetLogicalLevelNumber(int logicalLevel);
    static int ConvertLogicalToInternalLevel(int logicalLevel);
    static int ConvertInternalToLogicalLevel(int internalLevel);

private:
    static const int MAX_SIZE_X = 128;
    static const int MAX_SIZE_Z = 128;

    int t[MAX_SIZE_X][MAX_SIZE_Z];
    int f[MAX_SIZE_X][MAX_SIZE_Z];
    
    // JSON metadata support
    LevelMetadata metadata;
    bool LoadMetadata(const std::string& levelPath);
    std::string GetMetadataPath(const std::string& levelPath) const;
    
    // Debug method to print loaded metadata
    void PrintMetadata() const;

    GameWorld* gameWorld = nullptr; // Pointer to the GameWorld instance
};