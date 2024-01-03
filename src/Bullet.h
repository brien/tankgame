//
//  Bullet.hpp
//  tankgame
//
//

#ifndef Bullet_h
#define Bullet_h

class Tank;
enum class TankType;

class Bullet
{

public:
    Bullet();
    Bullet(int tid, float power,
           TankType type1, TankType type2,
           int maxbounces,
           float dTpressed,
           float r, float g, float b,
           float r2, float g2, float b2,
           float x, float y, float z,
           float rx, float ry, float rz);
    void NextFrame();
    void Draw();
    bool alive;

private:
    float x, y, z;
    float vx, vy, vz;
    float rx, ry, rz;
    float dty;
    
    float r, g, b;
    float r2, g2, b2;
    
    float moveRate;
    float power;
    
    bool isSpecial;
    
    int id;
    int tankId;
    
    TankType type1;
    TankType type2;
    
    float dT;
    
    float maxdT;
    
    int numbounces;
    int maxbounces;
    
    void HandleLevelCollision(float xpp, float zpp, float ory);
    void HandlePlayerCollision(Tank& playerTank);
    void HandleTankCollision(Tank& playerTank);
};


#endif /* Bullet_h */
