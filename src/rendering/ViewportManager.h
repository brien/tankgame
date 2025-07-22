#ifndef VIEWPORTMANAGER_H
#define VIEWPORTMANAGER_H

#include <vector>

/**
 * Viewport data structure representing a screen region for rendering
 */
struct Viewport {
    int x;          // Left edge of viewport in pixels
    int y;          // Bottom edge of viewport in pixels
    int width;      // Width of viewport in pixels
    int height;     // Height of viewport in pixels
    
    Viewport(int x = 0, int y = 0, int width = 800, int height = 600)
        : x(x), y(y), width(width), height(height) {}
    
    // Calculate aspect ratio for this viewport
    float GetAspectRatio() const {
        return height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
    }
    
    // Check if viewport is valid (non-zero area)
    bool IsValid() const {
        return width > 0 && height > 0;
    }
};

/**
 * Manages viewport layout for single-player and split-screen rendering
 * Replaces hardcoded viewport calculations in GraphicsTask
 */
class ViewportManager {
public:
    ViewportManager();
    ~ViewportManager() = default;
    
    // Configure viewports for different player counts
    void SetupSinglePlayer(int screenWidth, int screenHeight);
    void SetupSplitScreen(int numPlayers, int screenWidth, int screenHeight);
    
    // Set the active viewport for OpenGL rendering
    void SetActiveViewport(int playerIndex);
    
    // Get viewport information
    const Viewport& GetViewport(int playerIndex) const;
    int GetNumViewports() const { return static_cast<int>(viewports.size()); }
    
    // Check if current setup is split-screen
    bool IsSplitScreen() const { return viewports.size() > 1; }
    
    // Get screen dimensions
    int GetScreenWidth() const { return screenWidth; }
    int GetScreenHeight() const { return screenHeight; }
    
private:
    std::vector<Viewport> viewports;
    int currentViewport;
    int screenWidth;
    int screenHeight;
    
    // Helper methods for viewport calculation
    void CreateHorizontalSplitViewports(int screenWidth, int screenHeight);
    void CreateVerticalSplitViewports(int screenWidth, int screenHeight);
    void ValidateViewports();
};

#endif // VIEWPORTMANAGER_H
