#ifndef TANK_CLASS_H_INCLUDED
#define TANK_CLASS_H_INCLUDED

#include "igtl_qmesh.h"

#include <vector>
#include <queue>
using namespace std;
#include "bullet.h"

class Tank 
{
public:

	Tank();
	~Tank();

	vector<Bullet> bullets;
   queue<Bullet> bulletq;

   bool turbo;
   bool recharge;
	float charge;
	float maxCharge;
	float chargeRegen;

	float fireCost;
	float jumpCost;
	float moveCost;
   float chargeCost;

   float fireTimer;
   float fireRate;
   int bounces;
   int attack;

	bool alive;

	float energy;
	float maxEnergy;
	float energyRegen;

	int id;

	void Init();

	float x, y, z;
	float vx, vy, vz;
   //float vy;
	void SetPosition(float _x, float _y, float _z);

	float collisionPoints[21];
   float size;

	int control;
   unsigned int inputmode;
   unsigned int jid;

	//float DTR;
	//float x, y, z;
	float rx, ry, rz, rr, rrl;
	float rtx, rty, rtz;
	float rotRate, movRate, jumpRate;
	float fallRate;
	//void SetPosition(float _x, float _y, float _z);
	void Fall();
	void Jump();
	void HandleInput();
	void AI();

   void Hunt(Tank& player);
   void Fear();
   void Wander();


	void RotBody(bool forb);
   void RotBody(float rate);
	void RotTurret(float rate);
	void RotBarrel(bool forb);
	bool Move(float rate);
   bool Move(bool forb);
	void Fire(float dTpressed);
   void Special(float dTpressed);
	void Die();

   void SetType(int t1, int t2);

   int type1;
   int type2;

   float r;
   float g;
   float b;

   float r2;
   float g2;
   float b2;

   float dist;

   bool jumping;
   bool grounded;
   float jumpTime;

	bool PointCollision(float cx, float cy, float cz);

	void NextFrame();

	void Draw();
	void Draw2();

   int bonus;
   float bonusTime;
   float deadtime;
   float hitAlpha;
   int hitNum;

	
};

#endif