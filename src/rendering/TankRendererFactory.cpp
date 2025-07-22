#include "TankRendererFactory.h"
#include "TankRenderer.h"
#include "PlayerTankRendererImpl.h"
#include "EnemyTankRendererImpl.h"

std::unique_ptr<ITankRenderer> TankRendererFactory::CreateRenderer(RendererType type) {
    switch (type) {
        case RendererType::PLAYER_TANK:
            return std::make_unique<PlayerTankRendererImpl>();
            
        case RendererType::ENEMY_TANK:
            return std::make_unique<EnemyTankRendererImpl>();
            
        case RendererType::UNIFIED:
        default:
            return std::make_unique<TankRenderer>();
    }
}

std::unique_ptr<ITankRenderer> TankRendererFactory::CreateRenderer(bool isPlayer) {
    if (isPlayer) {
        return CreateRenderer(RendererType::PLAYER_TANK);
    } else {
        return CreateRenderer(RendererType::ENEMY_TANK);
    }
}

std::unique_ptr<ITankRenderer> TankRendererFactory::CreateUnifiedRenderer() {
    return CreateRenderer(RendererType::UNIFIED);
}
