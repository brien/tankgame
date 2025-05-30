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
    ~Bullet() = default;
    void NextFrame();
    void Draw() const;
    bool alive = true;

private:
    float x = 0.0f, y = 0.0f, z = 0.0f;
    float vx = 0.0f, vy = 0.0f, vz = 0.0f;
    float rx = 0.0f, ry = 0.0f, rz = 0.0f;
    float dty = 0.0f;

    float r = 0.5f, g = 0.5f, b = 0.5f;
    float r2 = 0.5f, g2 = 0.5f, b2 = 0.5f;

    float moveRate = 33.0f;
    float power = 0.0f;

    bool isSpecial = false;

    int id = 0;
    int tankId = 0;

    TankType type1;
    TankType type2;

    float dT = 0.0f;
    float maxdT = 100.0f;

    int numbounces = 0;
    int maxbounces = 0;

    void HandleLevelCollision(float xpp, float zpp, float ory);
    void HandlePlayerCollision(Tank &playerTank);
    void HandleTankCollision(Tank &playerTank);
};

#endif /* Bullet_h */
