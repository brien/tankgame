//
//  Bullet.hpp
//  tankgame
//
//

#ifndef Bullet_h
#define Bullet_h

class Bullet
{

public:
    Bullet();
    Bullet(int tid, float power,
           int type1, int type2,
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
    
    float movRate;
    float power;
    
    bool spec;
    
    int id;
    int tid;
    
    int type1;
    int type2;
    
    float dT;
    
    float maxdT;
    
    int numbounces;
    int maxbounces;
    
    void HandleLevelCollision(float xpp, float zpp, float ory);
};


#endif /* Bullet_h */
