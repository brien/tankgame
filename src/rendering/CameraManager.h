#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include "../Camera.h"
#include "../Tank.h"
#include <array>

/**
 * Camera modes for different gameplay scenarios
 */
enum class CameraMode {
    FOLLOW,          // Standard follow camera behind tank
    OVERHEAD,        // Top-down view
    FIRST_PERSON,    // First person view (future)
    FIXED            // Fixed position camera (future)
};

/**
 * Manages camera positioning and behavior for all players
 * Extracts camera logic from GraphicsTask for better organization
 */
class CameraManager {
public:
    static constexpr int MAX_CAMERAS = 4;
    static constexpr float DEFAULT_XZ_DISTANCE = 3.0f;
    static constexpr float DEFAULT_Y_DISTANCE = 2.0f;
    static constexpr float FOCUS_HEIGHT_OFFSET = 0.3f;
    
    CameraManager();
    ~CameraManager() = default;
    
    // Camera setup and management
    void InitializeCameras();
    void UpdateCameras(const std::array<Tank, 2>& players, int numPlayers, bool gameStarted);
    void UpdateCamera(int playerId, const Tank& tank);
    
    // Camera access
    Camera& GetCamera(int playerId);
    const Camera& GetCamera(int playerId) const;
    
    // Camera configuration
    void SetCameraMode(int playerId, CameraMode mode);
    CameraMode GetCameraMode(int playerId) const;
    
    void SetCameraDistance(int playerId, float xzDistance, float yDistance);
    void GetCameraDistance(int playerId, float& xzDistance, float& yDistance) const;
    
    // Utility methods
    bool IsValidPlayerId(int playerId) const;
    void ResetCamera(int playerId);
    void ResetAllCameras();
    
private:
    std::array<Camera, MAX_CAMERAS> cameras;
    std::array<CameraMode, MAX_CAMERAS> cameraModes;
    
    // Camera calculation methods
    void UpdateFollowCamera(int playerId, const Tank& tank);
    void UpdateOverheadCamera(int playerId, const Tank& tank);
    
    // Helper methods for camera positioning
    void CalculateCameraPosition(const Tank& tank, float xzDistance, float yDistance, 
                                float& camX, float& camY, float& camZ) const;
    void CalculateFocusPosition(const Tank& tank, float& focusX, float& focusY, float& focusZ) const;
    
    // Convert degrees to radians (DTR constant)
    static constexpr float DTR = 0.017453f;
};

#endif // CAMERAMANAGER_H
