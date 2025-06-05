#pragma once

#include "InputHandler.h"
#include "Tank.h"
#include <memory>

class InputHandlerFactory
{
public:
    static std::unique_ptr<InputHandler> CreateInputHandler(InputMode mode);
};
