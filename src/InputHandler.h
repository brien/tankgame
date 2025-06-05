#pragma once

// Forward declaration
class Tank;

// Base interface for input handling
class InputHandler
{
public:
    virtual ~InputHandler() = default;
    virtual void HandleInput(Tank& tank) = 0;
};
