#ifndef TANKHANDLER_H_INCLUDED
#define TANKHANDLER_H_INCLUDED

#include "Tank.h"
#include "Singleton.h"

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

    vector<Tank> tanks;

    void CreateTank();
    void DestroyTank();

    void NextFrame();

    void DrawTanks();
    void DrawBullets();

    float closest;

    bool isInputJoy;

    int diff;

    int numPlayers;
    int numAttackingTanks;

private:
    void InitializeEnemyTanks();
    void InitializePlayerTanks();
    void InitializePlayerControls();
};

#endif
