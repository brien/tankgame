#pragma once

/**
 * Example usage of the new ITankRenderer interface and TankRendererFactory.
 * 
 * This example demonstrates how to use the new tank rendering architecture
 * that was created in Phase 3, Step 9-11 of the refactoring plan.
 * 
 * The new architecture provides:
 * - Clean separation between rendering and game logic
 * - Polymorphic rendering through ITankRenderer interface
 * - Factory pattern for creating appropriate renderers
 * - Data-driven rendering using TankRenderData
 */

#include "ITankRenderer.h"
#include "TankRendererFactory.h"
#include "RenderData.h"
#include <vector>
#include <memory>

namespace TankRenderingExample {

    /**
     * Example function showing basic usage of the new tank rendering system.
     */
    void BasicTankRenderingExample() {
        // Create tank renderers using the factory
        auto unifiedRenderer = TankRendererFactory::CreateUnifiedRenderer();
        auto playerRenderer = TankRendererFactory::CreateRenderer(true);  // true = player
        auto enemyRenderer = TankRendererFactory::CreateRenderer(false);  // false = enemy
        
        // Initialize renderers
        if (!unifiedRenderer->Initialize() || 
            !playerRenderer->Initialize() || 
            !enemyRenderer->Initialize()) {
            // Handle initialization failure
            return;
        }
        
        // Example tank data
        std::vector<TankRenderData> tanks;
        
        // Create a player tank
        TankRenderData playerTank = {};
        playerTank.alive = true;
        playerTank.isPlayer = true;
        playerTank.position = {0.0f, 0.0f, 0.0f};
        playerTank.bodyRotation = {0.0f, 45.0f, 0.0f};
        playerTank.turretRotation = {0.0f, 90.0f, 0.0f};
        playerTank.r = 0.2f; playerTank.g = 0.8f; playerTank.b = 0.2f;  // Green primary color
        playerTank.r2 = 0.1f; playerTank.g2 = 0.6f; playerTank.b2 = 0.1f; // Dark green secondary
        playerTank.health = 80.0f;
        playerTank.maxHealth = 100.0f;
        playerTank.charge = 50.0f;
        playerTank.fireCost = 25.0f;
        tanks.push_back(playerTank);
        
        // Create an enemy tank
        TankRenderData enemyTank = {};
        enemyTank.alive = true;
        enemyTank.isPlayer = false;
        enemyTank.position = {5.0f, 0.0f, 5.0f};
        enemyTank.bodyRotation = {0.0f, -30.0f, 0.0f};
        enemyTank.turretRotation = {0.0f, 60.0f, 0.0f};
        enemyTank.r = 0.8f; enemyTank.g = 0.2f; enemyTank.b = 0.2f;  // Red primary color
        enemyTank.r2 = 0.6f; enemyTank.g2 = 0.1f; enemyTank.b2 = 0.1f; // Dark red secondary
        enemyTank.health = 60.0f;
        enemyTank.maxHealth = 100.0f;
        tanks.push_back(enemyTank);
        
        // Method 1: Use unified renderer for all tanks
        unifiedRenderer->RenderMultiple(tanks);
        
        // Method 2: Use specialized renderers for specific tank types
        for (const auto& tank : tanks) {
            if (tank.isPlayer) {
                playerRenderer->Render(tank);
            } else {
                enemyRenderer->Render(tank);
            }
        }
        
        // Clean up
        unifiedRenderer->Cleanup();
        playerRenderer->Cleanup();
        enemyRenderer->Cleanup();
    }
    
    /**
     * Example function showing advanced usage with custom rendering pipeline.
     */
    void AdvancedTankRenderingExample(const std::vector<TankRenderData>& tanks) {
        // Create renderer based on performance requirements
        auto renderer = TankRendererFactory::CreateRenderer(
            TankRendererFactory::RendererType::UNIFIED
        );
        
        if (!renderer->Initialize()) {
            // Handle initialization failure
            return;
        }
        
        // Custom rendering loop with proper state management
        renderer->SetupRenderState();
        
        // Render tanks in batches for efficiency
        std::vector<TankRenderData> playerTanks;
        std::vector<TankRenderData> enemyTanks;
        
        // Separate tanks by type for optimized rendering
        for (const auto& tank : tanks) {
            if (tank.alive) {
                if (tank.isPlayer) {
                    playerTanks.push_back(tank);
                } else {
                    enemyTanks.push_back(tank);
                }
            }
        }
        
        // Render player tanks first (they need more detail)
        for (const auto& tank : playerTanks) {
            renderer->Render(tank);
        }
        
        // Render enemy tanks in batch (simpler rendering)
        for (const auto& tank : enemyTanks) {
            renderer->Render(tank);
        }
        
        renderer->CleanupRenderState();
        renderer->Cleanup();
    }
    
    /**
     * Example function showing how to integrate with existing rendering pipeline.
     */
    void IntegrateWithExistingPipeline() {
        // This shows how the new system can be integrated into GraphicsTask
        // as part of Phase 4 of the refactoring plan
        
        auto tankRenderer = TankRendererFactory::CreateUnifiedRenderer();
        if (!tankRenderer->Initialize()) {
            // Handle initialization failure
            return;
        }
        
        // In GraphicsTask::Update(), you would use it like this:
        // 1. Extract tank data from game objects
        // std::vector<TankRenderData> tankData = sceneBuilder->ExtractTankData();
        
        // 2. Render tanks using the new interface
        // tankRenderer->RenderMultiple(tankData);
        
        // This replaces the old approach of calling TankHandler::Draw() directly
        // and provides clean separation between game logic and rendering
        
        tankRenderer->Cleanup();
    }
}

/* 
 * Summary of what was accomplished in Phase 3, Steps 9-11:
 * 
 * Step 9: ✓ Created ITankRenderer interface
 * - Defined clean contract for all tank renderers
 * - Supports both single and batch rendering
 * - Provides proper lifecycle management (Initialize/Cleanup)
 * - Includes render state management methods
 * 
 * Step 10: ✓ Refactored existing tank renderers to implement ITankRenderer
 * - NewTankRenderer now implements ITankRenderer (unified renderer)
 * - Created PlayerTankRendererImpl (specialized for player tanks)
 * - Created EnemyTankRendererImpl (optimized for enemy tanks)
 * - Maintained backward compatibility with legacy wrapper methods
 * 
 * Step 11: ✓ Created TankRendererFactory
 * - Factory pattern for creating appropriate renderers
 * - Supports different renderer types (player, enemy, unified)
 * - Convenience methods for easy renderer creation
 * - Smart pointer return values for memory safety
 * 
 * Next Steps (Phase 4):
 * - Integrate ITankRenderer into centralized rendering pipeline
 * - Create scene data builder to extract TankRenderData from game objects
 * - Remove OpenGL code from TankHandler
 * - Update GraphicsTask to use new architecture
 */
