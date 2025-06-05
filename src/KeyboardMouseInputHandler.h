#pragma once

#include "InputHandler.h"

class KeyboardMouseInputHandler : public InputHandler
{
public:
    void HandleInput(Tank& tank) override;
};
