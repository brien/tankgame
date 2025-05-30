#pragma once

#include <array>
#include <vector>
#include "Tank.h"
#include "Singleton.h"

class TankHandler : public Singleton<TankHandler>
{
public:
    TankHandler();
    ~TankHandler() = default;

    void Init();

    std::array<Tank, 4> players{};
    std::array<float, 2> combo{};
    std::array<float, 2> special{};
    std::array<int, 2> comboNum{};
    std::array<int, 2> hitCombo{};
    std::array<int, 2> wins{};

    std::vector<Tank> tanks;

    void CreateTank();
    void DestroyTank();

    void NextFrame();

    void DrawTanks() const;
    void DrawBullets() const;

    float closest = 0.0f;
    bool isInputJoy = false;
    int difficultySetting = 0;
    int numPlayers = 1;
    int numAttackingTanks = 0;

private:
    void InitializeEnemyTanks();
    void InitializePlayerTanks();
    void InitializePlayerControls();
};
