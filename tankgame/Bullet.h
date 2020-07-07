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
    float x, y, z;
    float vx, vy, vz;
    float rx, ry, rz;
    float dty;
    
    float r, g, b;
    float r2, g2, b2;
    
    float movRate;
    float power;
    
    void NextFrame();
    void Draw();
    
    bool alive;
    bool spec;
    
    int id;
    int tid;
    
    int type1;
    int type2;
    
    float dT;
    
    float maxdT;
    
    int numbounces;
    int maxbounces;
    
};


#endif /* Bullet_h */
