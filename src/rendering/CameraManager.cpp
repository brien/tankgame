#include "CameraManager.h"
#include "../math.h"
#include <cmath>
#include <cassert>

CameraManager::CameraManager() {
    InitializeCameras();
}

void CameraManager::InitializeCameras() {
    for (int i = 0; i < MAX_CAMERAS; ++i) {
        // Set default camera distances (matching original GraphicsTask behavior)
        cameras[i].xzdist = DEFAULT_XZ_DISTANCE;
        cameras[i].ydist = DEFAULT_Y_DISTANCE;
        
        // Set default camera mode
        cameraModes[i] = CameraMode::FOLLOW;
        
        // Initialize position and focus to origin
        cameras[i].SetPos(0.0f, DEFAULT_Y_DISTANCE, -DEFAULT_XZ_DISTANCE);
        cameras[i].SetFocus(0.0f, 0.0f, 0.0f);
    }
}

void CameraManager::UpdateCameras(const std::array<Tank, 2>& players, int numPlayers, bool gameStarted) {
    if (!gameStarted) {
        return;
    }
    
    // Update cameras for all active players
    for (int i = 0; i < numPlayers && i < MAX_CAMERAS; ++i) {
        UpdateCamera(i, players[i]);
    }
}

void CameraManager::UpdateCamerasFromPointers(const std::array<Tank*, 2>& players, int numPlayers, bool gameStarted) {
    if (!gameStarted) {
        return;
    }
    
    // Update cameras for all active players
    for (int i = 0; i < numPlayers && i < MAX_CAMERAS; ++i) {
        if (players[i]) {
            UpdateCamera(i, *players[i]);
        }
    }
}

void CameraManager::UpdateCamera(int playerId, const Tank& tank) {
    if (!IsValidPlayerId(playerId)) {
        return;
    }
    
    switch (cameraModes[playerId]) {
        case CameraMode::FOLLOW:
            UpdateFollowCamera(playerId, tank);
            break;
        case CameraMode::OVERHEAD:
            UpdateOverheadCamera(playerId, tank);
            break;
        default:
            UpdateFollowCamera(playerId, tank);
            break;
    }
}

void CameraManager::UpdateFollowCamera(int playerId, const Tank& tank) {
    Camera& cam = cameras[playerId];
    
    // Calculate camera position (behind the tank based on rotation)
    float camX, camY, camZ;
    CalculateCameraPosition(tank, cam.xzdist, cam.ydist, camX, camY, camZ);
    cam.SetPos(camX, camY, camZ);
    
    // Calculate focus position (in front of the tank)
    float focusX, focusY, focusZ;
    CalculateFocusPosition(tank, focusX, focusY, focusZ);
    cam.SetFocus(focusX, focusY, focusZ);
}

void CameraManager::UpdateOverheadCamera(int playerId, const Tank& tank) {
    Camera& cam = cameras[playerId];
    
    // Position camera directly above the tank
    cam.SetPos(tank.x, tank.y + cam.ydist * 2.0f, tank.z);
    
    // Focus on the tank itself
    cam.SetFocus(tank.x, tank.y, tank.z);
}

void CameraManager::CalculateCameraPosition(const Tank& tank, float xzDistance, float yDistance,
                                           float& camX, float& camY, float& camZ) const {
    // Calculate combined rotation (body + turret)
    float totalRotation = (tank.ry + tank.rty) * DTR;
    
    // Position camera behind the tank
    camX = tank.x - xzDistance * std::cos(totalRotation);
    camY = tank.y + yDistance;
    camZ = tank.z - xzDistance * std::sin(totalRotation);
}

void CameraManager::CalculateFocusPosition(const Tank& tank, float& focusX, float& focusY, float& focusZ) const {
    // Calculate combined rotation (body + turret)
    float totalRotation = (tank.ry + tank.rty) * DTR;
    
    // Focus point in front of the tank
    focusX = tank.x + std::cos(totalRotation);
    focusY = tank.y + FOCUS_HEIGHT_OFFSET;
    focusZ = tank.z + std::sin(totalRotation);
}

Camera& CameraManager::GetCamera(int playerId) {
    assert(IsValidPlayerId(playerId));
    return cameras[playerId];
}

const Camera& CameraManager::GetCamera(int playerId) const {
    assert(IsValidPlayerId(playerId));
    return cameras[playerId];
}

void CameraManager::SetCameraMode(int playerId, CameraMode mode) {
    if (IsValidPlayerId(playerId)) {
        cameraModes[playerId] = mode;
    }
}

CameraMode CameraManager::GetCameraMode(int playerId) const {
    if (IsValidPlayerId(playerId)) {
        return cameraModes[playerId];
    }
    return CameraMode::FOLLOW;
}

void CameraManager::SetCameraDistance(int playerId, float xzDistance, float yDistance) {
    if (IsValidPlayerId(playerId)) {
        cameras[playerId].xzdist = xzDistance;
        cameras[playerId].ydist = yDistance;
    }
}

void CameraManager::GetCameraDistance(int playerId, float& xzDistance, float& yDistance) const {
    if (IsValidPlayerId(playerId)) {
        xzDistance = cameras[playerId].xzdist;
        yDistance = cameras[playerId].ydist;
    } else {
        xzDistance = DEFAULT_XZ_DISTANCE;
        yDistance = DEFAULT_Y_DISTANCE;
    }
}

bool CameraManager::IsValidPlayerId(int playerId) const {
    return playerId >= 0 && playerId < MAX_CAMERAS;
}

void CameraManager::ResetCamera(int playerId) {
    if (IsValidPlayerId(playerId)) {
        cameras[playerId].xzdist = DEFAULT_XZ_DISTANCE;
        cameras[playerId].ydist = DEFAULT_Y_DISTANCE;
        cameraModes[playerId] = CameraMode::FOLLOW;
        cameras[playerId].SetPos(0.0f, DEFAULT_Y_DISTANCE, -DEFAULT_XZ_DISTANCE);
        cameras[playerId].SetFocus(0.0f, 0.0f, 0.0f);
    }
}

void CameraManager::ResetAllCameras() {
    for (int i = 0; i < MAX_CAMERAS; ++i) {
        ResetCamera(i);
    }
}
