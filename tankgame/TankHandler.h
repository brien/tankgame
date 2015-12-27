#ifndef TANKHANDLER_H_INCLUDED
#define TANKHANDLER_H_INCLUDED

#include "tank.h"
#include "singleton.h"

class TankHandler : public Singleton<TankHandler>
{
public:
	TankHandler();
	~TankHandler();

	void Init();
    
   Tank players[4];
   float combo[2];
   float special[2];
   int comboNum[2];
   int hitCombo[2];
   int wins[2];
   //nk player2;

	vector<Tank> tanks;

	void CreateTank();
	void DestroyTank();

	void NextFrame();

	void DrawTanks();
	void DrawBullets();

	float closest;

   bool isInputJoy;

   int diff;

   //bool twop;
   int numPlayers;
   int attackingTanks;
   
   igtl_QGLMesh bodymesh;
   igtl_QGLMesh turretmesh;
   igtl_QGLMesh cannonmesh;

};

#endif