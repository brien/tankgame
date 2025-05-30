#ifndef TANKHANDLER_H_INCLUDED
#define TANKHANDLER_H_INCLUDED

#include <array>
#include "Tank.h"
#include "Singleton.h"

class TankHandler : public Singleton<TankHandler>
{
public:
    TankHandler();
    ~TankHandler();

    void Init();

    std::array<Tank, 4> players;
    std::array<float, 2> combo;
    std::array<float, 2> special;
    std::array<int, 2> comboNum;
    std::array<int, 2> hitCombo;
    std::array<int, 2> wins;

    vector<Tank> tanks;

    void CreateTank();
    void DestroyTank();

    void NextFrame();

    void DrawTanks();
    void DrawBullets();

    float closest;

    bool isInputJoy;

    int difficultySetting;

    int numPlayers;
    int numAttackingTanks;

private:
    void InitializeEnemyTanks();
    void InitializePlayerTanks();
    void InitializePlayerControls();
};

#endif
