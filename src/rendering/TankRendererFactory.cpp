#include "TankRendererFactory.h"
#include "TankRenderer.h"
#include "PlayerTankRendererImpl.h"
#include "EnemyTankRendererImpl.h"

std::unique_ptr<ITankRenderer> TankRendererFactory::CreateRenderer(RendererType type) {
    switch (type) {
        case RendererType::PLAYER_TANK:
            return std::make_unique<PlayerTankRendererImpl>();
            
        case RendererType::ENEMY_TANK:
#ifdef __EMSCRIPTEN__
            // Enemy rendering is part of the deliberately deferred browser
            // tank renderer. Keep factory callers valid without linking the
            // desktop immediate-mode implementation.
            return std::make_unique<TankRenderer>();
#else
            return std::make_unique<EnemyTankRendererImpl>();
#endif
            
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
