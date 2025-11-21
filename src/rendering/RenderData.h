#ifndef RENDERDATA_H
#define RENDERDATA_H

#include <vector>

// Include Color.h for Color struct
#include "../Color.h"
// Include Tank.h for TankType enum
#include "../Tank.h"
// Include FXHandler.h for FxType enum
#include "../FXHandler.h"

// Forward declaration for UI data
struct UIRenderData;

/**
 * Basic 3D vector structure for rendering data
 */
struct Vector3 {
    float x, y, z;
    
    Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}
    
    Vector3(const Vector3& other) : x(other.x), y(other.y), z(other.z) {}
    
    Vector3& operator=(const Vector3& other) {
        if (this != &other) {
            x = other.x;
            y = other.y;
            z = other.z;
        }
        return *this;
    }
};

/**
 * Complete rendering data for a tank, extracted from Tank class
 * Contains all visual state needed for rendering without game logic
 */
struct TankRenderData {
    // Position and rotation
    Vector3 position;           // x, y, z world coordinates
    Vector3 bodyRotation;       // rx, ry, rz body orientation
    Vector3 turretRotation;     // rtx, rty, rtz turret orientation (independent of body)
    float targetRotation;       // rrl - rotation towards closest enemy for targeting UI
    
    // Visual state for HUD and effects
    float health;               // Current health for health bar
    float maxHealth;            // Maximum health for health bar percentage
    float charge;               // Current charge for charge bar
    float maxCharge;            // Maximum charge for charge bar percentage
    float fireCost;             // Cost to fire (for charge indicator line)
    
    // Tank colors
    Color primaryColor;              // Primary color (0.0 - 1.0)
    Color secondaryColor;           // Secondary color (0.0 - 1.0)
    
    // Status indicators
    bool alive;                 // Whether tank should be rendered
    bool isPlayer;              // Different rendering for player vs enemy tanks
    int playerId;               // -1, -2 for players, positive for enemies
    
    // Special effects data
    int bonus;                  // Power-up type to display as icon
    float bonusTime;            // Time remaining for bonus (for animation)
    float hitAlpha;             // Alpha value for hit flash effect (0.0 - 1.0)
    int hitNum;                 // Combo hit number for display
    
    // Tank type info (determines mesh and special rendering)
    TankType type1, type2;      // Primary and secondary tank types
    
    // Movement state (for particle effects and animations)
    bool grounded;              // Whether tank is on ground (for tread marks)
    bool isJumping;             // Whether tank is jumping (for jump effects)
    Vector3 velocity;           // vx, vy, vz for motion-based effects
    
    // Constructor with sensible defaults
    TankRenderData() 
        : position(0, 0, 0)
        , bodyRotation(0, 0, 0)
        , turretRotation(0, 0, 0)
        , targetRotation(0.0f)
        , health(100.0f)
        , maxHealth(100.0f)
        , charge(0.0f)
        , maxCharge(100.0f)
        , fireCost(10.0f)
        , primaryColor(1.0f, 1.0f, 1.0f, 1.0f)
        , secondaryColor(0.5f, 0.5f, 0.5f, 1.0f)
        , alive(true)
        , isPlayer(false)
        , playerId(0)
        , bonus(0)
        , bonusTime(0.0f)
        , hitAlpha(0.0f)
        , hitNum(0)
        , type1(TankType::TYPE_GREY)
        , type2(TankType::TYPE_GREY)
        , grounded(true)
        , isJumping(false)
        , velocity(0, 0, 0)
    {}
};

/**
 * Rendering data for bullets/projectiles
 * Contains visual state and motion data for bullet rendering
 */
struct BulletRenderData {
    Vector3 position;           // Current bullet position
    Vector3 rotation;           // Bullet orientation for non-spherical bullets
    Vector3 velocity;           // Motion vector for trail effects
    
    // Colors inherited from firing tank
    Color primaryColor;         // Primary color
    Color secondaryColor;       // Secondary color
    
    // Type information for specialized bullet rendering
    TankType type1, type2;      // Bullet type determines appearance
    int ownerId;                // ID of tank that fired this bullet
    float speed;                // Current speed for trail length calculation
    float power;                // Bullet power (affects alpha blending)
    
    // Constructor
    BulletRenderData()
        : position(0, 0, 0)
        , rotation(0, 0, 0)
        , velocity(0, 0, 0)
        , primaryColor(1.0f, 1.0f, 1.0f, 1.0f)
        , secondaryColor(0.5f, 0.5f, 0.5f, 1.0f)
        , type1(TankType::TYPE_GREY)
        , type2(TankType::TYPE_GREY)
        , ownerId(-1)
        , speed(0.0f)
        , power(100.0f)
    {}
};

/**
 * Rendering data for visual effects (explosions, smoke, particles)
 */
struct EffectRenderData {
    FxType type;                // TYPE_DEATH, TYPE_SMOKE, etc.
    Vector3 position;           // Effect center position
    Vector3 rotation;           // Effect orientation
    Vector3 velocity;           // Effect motion (for moving effects)
    
    // Visual properties
    float r, g, b;              // Effect color
    float alpha;                // Transparency (0.0 - 1.0)
    float scale;                // Size multiplier
    float lifetime;             // Time remaining for fade calculations
    
    // Constructor
    EffectRenderData()
        : type(FxType::TYPE_ZERO)
        , position(0, 0, 0)
        , rotation(0, 0, 0)
        , velocity(0, 0, 0)
        , r(1.0f), g(1.0f), b(1.0f)
        , alpha(1.0f)
        , scale(1.0f)
        , lifetime(1.0f)
    {}
};

/**
 * Rendering data for power-up items
 */
struct ItemRenderData {
    Vector3 position;           // Item world position
    TankType itemType;          // Determines which power-up mesh to use
    float rotationY;            // Y-axis rotation for spinning animation
    bool visible;               // Whether item should be rendered (not picked up)
    
    // Constructor
    ItemRenderData()
        : position(0, 0, 0)
        , itemType(TankType::TYPE_GREY)
        , rotationY(0.0f)
        , visible(true)
    {}
};

/**
 * Rendering data for terrain/level geometry
 */
struct TerrainRenderData {
    static const int MAX_SIZE_X = 128;
    static const int MAX_SIZE_Z = 128;
    
    int levelNumber;                        // Different levels have different terrain meshes
    int colorNumber;                        // Primary color index
    int colorNumber2;                       // Secondary color index
    int sizeX;                             // Terrain width
    int sizeZ;                             // Terrain depth
    
    // Terrain height maps
    int heightMap[MAX_SIZE_X][MAX_SIZE_Z];     // Main terrain height data (t array)
    int floatMap[MAX_SIZE_X][MAX_SIZE_Z];      // Floating elements height data (f array)
    
    // Drawing flags
    bool drawFloor;
    bool drawWalls;
    bool drawTop;
    
    // Terrain colors (moved from LevelHandler OpenGL calls)
    struct TerrainColors {
        Vector3 defaultColor;    // Level 48: White (1.0, 1.0, 1.0)
        Vector3 specialColor1;   // Level 49: Green (0.1, 1.0, 0.1)
        Vector3 specialColor2;   // Level 50: Red (1.0, 0.1, 0.1)
        Vector3 blockColor;      // Block rendering: Green (0.0, 1.0, 0.0)
    } colors;
    
    // Constructor
    TerrainRenderData() : 
        levelNumber(0), 
        colorNumber(0), 
        colorNumber2(0), 
        sizeX(128), 
        sizeZ(128),
        drawFloor(true),
        drawWalls(false),
        drawTop(false),
        colors{
            Vector3(1.0f, 1.0f, 1.0f),  // defaultColor - white
            Vector3(0.1f, 1.0f, 0.1f),  // specialColor1 - green
            Vector3(1.0f, 0.1f, 0.1f),  // specialColor2 - red
            Vector3(0.0f, 1.0f, 0.0f)   // blockColor - green
        }
    {
        // Initialize arrays to zero
        for (int x = 0; x < MAX_SIZE_X; x++) {
            for (int z = 0; z < MAX_SIZE_Z; z++) {
                heightMap[x][z] = 0;
                floatMap[x][z] = 0;
            }
        }
    }
};

/**
 * Camera data for each player viewport
 */
struct CameraData {
    Vector3 position;           // Camera world position
    Vector3 focus;              // Camera look-at target
    int playerId;               // Which player this camera follows
    
    // Constructor
    CameraData() : position(0, 0, 0), focus(0, 0, 0), playerId(0) {}
};

/**
 * Complete scene data containing all objects to be rendered
 * This structure separates rendering data from game logic
 */
struct SceneData {
    // Renderable objects
    std::vector<TankRenderData> tanks;
    std::vector<BulletRenderData> bullets;
    std::vector<EffectRenderData> effects;
    std::vector<ItemRenderData> items;
    TerrainRenderData terrain;
    
    // Sky rendering (special case for level 48)
    bool drawSky;
    
    // Camera information for split-screen rendering
    std::vector<CameraData> cameras;
    
    // UI rendering data (menus, HUD, debug info)
    UIRenderData* uiData;           // Pointer to avoid circular includes
    
    // Game state needed for rendering decisions
    int numPlayers;             // Number of active players (affects viewport layout)
    bool gameStarted;           // Whether game is in progress
    bool paused;                // Whether game is paused
    bool versusMode;            // Whether in versus mode
    bool debugMode;             // Whether to show debug info (FPS, etc.)
    
    // Constructor with defaults
    SceneData()
        : drawSky(false)
        , uiData(nullptr)
        , numPlayers(1)
        , gameStarted(false)
        , paused(false)
        , versusMode(false)
        , debugMode(false)
    {
        cameras.reserve(2);  // Reserve space for up to 2 player cameras
        tanks.reserve(10);   // Reserve reasonable space for tanks
        bullets.reserve(50); // Reserve space for bullets
        effects.reserve(20); // Reserve space for effects
        items.reserve(10);   // Reserve space for items
    }
    
    // Clear all dynamic data (for reuse)
    void Clear() {
        tanks.clear();
        bullets.clear();
        effects.clear();
        items.clear();
        cameras.clear();
    }
};

#endif // RENDERDATA_H
