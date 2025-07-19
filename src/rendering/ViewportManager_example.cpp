/*
 * ViewportManager Usage Example
 * 
 * This demonstrates how ViewportManager replaces the hardcoded viewport logic
 * from GraphicsTask::Update()
 * 
 * OLD CODE (from GraphicsTask.cpp):
 * ================================
 * if(TankHandler::GetSingleton().numPlayers>1) {
 *     glViewport(0, 0, VideoTask::scrWidth, VideoTask::scrHeight/2);
 *     // ... render for player 1
 *     glViewport(0, 300, VideoTask::scrWidth, VideoTask::scrHeight/2);  // HARDCODED!
 *     // ... render for player 0
 * }
 * 
 * NEW CODE (using ViewportManager):
 * ================================
 * // Setup (done once when player count changes):
 * viewportManager.SetupSplitScreen(numPlayers, screenWidth, screenHeight);
 * 
 * // Rendering loop:
 * for (int playerIndex = 0; playerIndex < numPlayers; ++playerIndex) {
 *     viewportManager.SetActiveViewport(playerIndex);
 *     // ... render scene for this player
 * }
 * 
 * BENEFITS:
 * ========
 * 1. No hardcoded viewport coordinates (300, etc.)
 * 2. Proper calculation based on actual screen dimensions
 * 3. Easily extensible for different split-screen layouts
 * 4. Centralized viewport management
 * 5. Better encapsulation and testability
 */

#include "ViewportManager.h"

void ExampleUsage() {
    ViewportManager viewportManager;
    
    // Single player setup
    viewportManager.SetupSinglePlayer(800, 600);
    
    // Split screen setup for 2 players
    viewportManager.SetupSplitScreen(2, 800, 600);
    
    // Render for each player
    for (int i = 0; i < viewportManager.GetNumViewports(); ++i) {
        viewportManager.SetActiveViewport(i);
        
        const Viewport& vp = viewportManager.GetViewport(i);
        // vp.x, vp.y, vp.width, vp.height are now available
        
        // Render scene for this viewport...
    }
}
