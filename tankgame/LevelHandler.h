//
//  LevelHandler.h
//  tankgame
//
//  Created by Brien Smith Martínez on 27/12/15.
//  Copyright © 2015 Brien Smith Martínez. All rights reserved.
//

#ifndef LEVELHANDLER_H_INCLUDED
#define LEVELHANDLER_H_INCLUDED

#include <vector>
using namespace std;
#include "Item.h"
#include "Singleton.h"

class LevelHandler : public Singleton<LevelHandler>
{
public:
    LevelHandler();
    ~LevelHandler();
    
    vector<Item> items;
    
    void Init();
    
    bool Load(const char filepath[]);
    
    bool drawFloor;
    bool drawWalls;
    bool drawTop;
    
    int t[128][128];
    int f[128][128];
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
    void GenerateTerrain();
    
    void DrawTerrain();
    void DrawItems();
    
    void DrawTerrain_OLD();
    
    
    int Strlen(const char * stringy);
    
    void NextLevel(bool forb);
    
    int levelNumber;
    int colorNumber;
    int colorNumber2;
    
};

#endif