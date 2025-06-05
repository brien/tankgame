#include "InputHandlerFactory.h"
#include "KeyboardMouseInputHandler.h"
#include "GenericJoystickInputHandler.h"
#include "GameCubeInputHandler.h"
#include <memory>

std::unique_ptr<InputHandler> InputHandlerFactory::CreateInputHandler(InputMode mode)
{
    switch (mode)
    {
        case InputMode::MODE_KEYBOARD_MOUSE:
            return std::make_unique<KeyboardMouseInputHandler>();
        
        case InputMode::MODE_JOYSTICK_GENERIC:
        case InputMode::MODE_EXTREME_3D:
        case InputMode::MODE_OTHER:
            return std::make_unique<GenericJoystickInputHandler>();
        
        case InputMode::MODE_NINTENDO_GC:
            return std::make_unique<GameCubeInputHandler>();
        
        default:
            return std::make_unique<KeyboardMouseInputHandler>(); // Default fallback
    }
}
