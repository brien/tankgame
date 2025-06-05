#pragma once

#include "InputHandler.h"

class GameCubeInputHandler : public InputHandler
{
public:
    void HandleInput(Tank& tank) override;
};
