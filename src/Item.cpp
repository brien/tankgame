#ifdef _WIN32
// If building in windows:
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
// If building on macOS:
#include <OpenGL/gl.h>
#else
// If building on Linux:
#include <GL/gl.h>
#endif

#include "Item.h"
#include "App.h"

Item::Item()
{
    x = y = z = 0;
    rx = ry = rz = 0;
    alive = true;
}

Item::Item(float nx, float ny, float nz, TankType ntype)
{
    x = nx;
    y = ny;
    z = nz;
    rx = ry = rz = 0;
    type = ntype;
    alive = true;

    switch (ntype)
    {
    case TankType::TYPE_GREY:
        r = .5;
        g = .5;
        b = .5;
        break;
    case TankType::TYPE_RED:
        r = 1;
        g = 0;
        b = 0;
        break;
    case TankType::TYPE_BLUE:
        r = 0;
        g = 0;
        b = 1;
        break;
    case TankType::TYPE_YELLOW:
        r = 1;
        g = 1;
        b = 0;
        break;
    case TankType::TYPE_PURPLE:
        r = 1;
        g = 0;
        b = 1;
        break;
    default:
        r = .5;
        g = .5;
        b = .5;
    }
}

void Item::Update()
{
    // Update rotation for spinning animation
    ry++;
}