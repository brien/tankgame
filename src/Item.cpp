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
    color = Color(0.5f, 0.5f, 0.5f, 1.0f);  // Default grey
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
        color = Color(0.5f, 0.5f, 0.5f, 1.0f);
        break;
    case TankType::TYPE_RED:
        color = Color(1.0f, 0.0f, 0.0f, 1.0f);
        break;
    case TankType::TYPE_BLUE:
        color = Color(0.0f, 0.0f, 1.0f, 1.0f);
        break;
    case TankType::TYPE_YELLOW:
        color = Color(1.0f, 1.0f, 0.0f, 1.0f);
        break;
    case TankType::TYPE_PURPLE:
        color = Color(1.0f, 0.0f, 1.0f, 1.0f);
        break;
    default:
        color = Color(0.5f, 0.5f, 0.5f, 1.0f);
    }
}

void Item::Update()
{
    // Update rotation for spinning animation
    ry++;
}