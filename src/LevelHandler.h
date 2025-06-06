//
//  LevelHandler.h
//  tankgame
//
//

#pragma once

#include <vector>
using namespace std;
#include "Item.h"
#include "Singleton.h"

class LevelHandler : public Singleton<LevelHandler>
{
public:
    LevelHandler();
    ~LevelHandler() = default;

    vector<Item> items;

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
    void AddItem(float x, float y, float z, TankType type);

    bool HandlePointCollision(float &x, float &y, float &z, float &vx, float &vz);
    bool FallCollision(float x, float y, float z);
    bool FloatCollision(float x, float y, float z);
    int GetTerrainHeight(int x, int z);
    int GetFloatHeight(int x, int z);
    bool SetTerrainHeight(int x, int z, int height);
    void GenerateTerrain();

    void DrawTerrain();
    void DrawItems();

    void DrawTerrain_OLD();

    int Strlen(const char *stringy);

    void NextLevel(bool forb);

    int levelNumber;
    int colorNumber;
    int colorNumber2;

private:
    static const int MAX_SIZE_X = 128;
    static const int MAX_SIZE_Z = 128;

    int t[MAX_SIZE_X][MAX_SIZE_Z];
    int f[MAX_SIZE_X][MAX_SIZE_Z];
};