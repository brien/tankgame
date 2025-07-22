#pragma once

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

class Tank;

class TankRendererDeprecated {
    // This class is deprecated and should not be used.
    // It serves as a placeholder to maintain compatibility with old code.
public:
    static void Draw(const Tank& tank);
    static void Draw2(const Tank& tank);
    
private:
    static void DrawBody(float r, float g, float b, float energy, float maxEnergy);
    static void DrawBarrel(float r, float g, float b, float energy, float maxEnergy);
    static void DrawTurret();
    
    static void SetupBodyTransform(const Tank& tank);
    static void SetupBarrelTransform(const Tank& tank);
    static void SetupTurretTransform(const Tank& tank);
};
