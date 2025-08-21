#include "ViewportManager.h"
#include <algorithm>
#include <cassert>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

ViewportManager::ViewportManager()
    : currentViewport(0), screenWidth(800), screenHeight(600)
{
    // Initialize with single default viewport
    viewports.clear();
    viewports.reserve(2); // Maximum 2 players supported
}

void ViewportManager::SetupSinglePlayer(int screenWidth, int screenHeight)
{
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;

    viewports.clear();
    viewports.emplace_back(0, 0, screenWidth, screenHeight);

    currentViewport = 0;
    ValidateViewports();
}

void ViewportManager::SetupSplitScreen(int numPlayers, int screenWidth, int screenHeight)
{
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;

    viewports.clear();

    if (numPlayers <= 1)
    {
        SetupSinglePlayer(screenWidth, screenHeight);
        return;
    }

    // Currently support up to 2 players
    numPlayers = std::min(numPlayers, 2);

    if (numPlayers == 2)
    {
        CreateHorizontalSplitViewports(screenWidth, screenHeight);
    }

    currentViewport = 0;
    ValidateViewports();
}

void ViewportManager::SetActiveViewport(int playerIndex)
{
    if (playerIndex >= 0 && playerIndex < static_cast<int>(viewports.size()))
    {
        currentViewport = playerIndex;
        const Viewport &viewport = viewports[playerIndex];

        // Set OpenGL viewport
        glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
    }
}

const Viewport &ViewportManager::GetViewport(int playerIndex) const
{
    assert(playerIndex >= 0 && playerIndex < static_cast<int>(viewports.size()));
    return viewports[playerIndex];
}

void ViewportManager::CreateHorizontalSplitViewports(int screenWidth, int screenHeight)
{
    // Split screen horizontally (one above the other)
    // This matches the current GraphicsTask behavior:
    // Player 1 (index 1 in original code): top half
    // Player 0 (index 0 in original code): bottom half

    int halfHeight = screenHeight / 2;

    // Player 0 viewport (bottom half)
    viewports.emplace_back(0, 0, screenWidth, halfHeight);

    // Player 1 viewport (top half)
    // Note: Original code uses hardcoded y=300, but we calculate it properly
    viewports.emplace_back(0, halfHeight, screenWidth, halfHeight);
}

void ViewportManager::CreateVerticalSplitViewports(int screenWidth, int screenHeight)
{
    // Split screen vertically (side by side)
    // Currently unused but included for future expansion

    int halfWidth = screenWidth / 2;

    // Player 0 viewport (left half)
    viewports.emplace_back(0, 0, halfWidth, screenHeight);

    // Player 1 viewport (right half)
    viewports.emplace_back(halfWidth, 0, halfWidth, screenHeight);
}

void ViewportManager::ValidateViewports()
{
    // Remove any invalid viewports
    viewports.erase(
        std::remove_if(viewports.begin(), viewports.end(),
                       [](const Viewport &vp)
                       { return !vp.IsValid(); }),
        viewports.end());

    // Ensure we have at least one viewport
    if (viewports.empty())
    {
        viewports.emplace_back(0, 0, screenWidth, screenHeight);
    }

    // Clamp current viewport index
    if (currentViewport >= static_cast<int>(viewports.size()))
    {
        currentViewport = 0;
    }
}
