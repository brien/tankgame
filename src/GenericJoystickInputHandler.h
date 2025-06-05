#pragma once

#include "InputHandler.h"

class GenericJoystickInputHandler : public InputHandler
{
public:
    void HandleInput(Tank& tank) override;
};
