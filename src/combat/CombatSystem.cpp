#include "CombatSystem.h"
#include "../events/Events.h"
#include "../Bullet.h"
#include "../Tank.h"
#include "../TankHandler.h"
#include "../PlayerManager.h"
#include "../App.h"
#include <cmath>

void CombatSystem::Initialize() {
    // Subscribe to collision events
    Events::GetBus().Subscribe<BulletCollisionEvent>([this](const BulletCollisionEvent& event) {
        OnBulletCollision(event);
    });
    
    Events::GetBus().Subscribe<BulletLevelCollisionEvent>([this](const BulletLevelCollisionEvent& event) {
        OnBulletLevelCollision(event);
    });
    
    Events::GetBus().Subscribe<BulletOutOfBoundsEvent>([this](const BulletOutOfBoundsEvent& event) {
        OnBulletOutOfBounds(event);
    });
    
    Events::GetBus().Subscribe<BulletTimeoutEvent>([this](const BulletTimeoutEvent& event) {
        OnBulletTimeout(event);
    });
}

void CombatSystem::Shutdown() {
    // Event system will automatically unsubscribe when destroyed
}

void CombatSystem::OnBulletCollision(const BulletCollisionEvent& event) {
    Bullet* bullet = dynamic_cast<Bullet*>(event.bullet);
    Tank* tank = dynamic_cast<Tank*>(event.target);
    
    if (!bullet || !tank) return;
    
    // Determine if it's a player or enemy tank
    bool isPlayer = tank->isPlayer;
    
    if (isPlayer) {
        HandleBulletPlayerCollision(bullet, tank);
    } else {
        HandleBulletTankCollision(bullet, tank);
    }
    
    // Mark bullet as dead
    bullet->Kill();
}

void CombatSystem::OnBulletLevelCollision(const BulletLevelCollisionEvent& event) {
    Bullet* bullet = dynamic_cast<Bullet*>(event.bullet);
    if (!bullet) return;
    
    // Delegate to bullet's existing level collision logic for now
    bullet->HandleLevelCollision(event.xMovement, event.zMovement, event.originalAngle);
}

void CombatSystem::OnBulletOutOfBounds(const BulletOutOfBoundsEvent& event) {
    Bullet* bullet = dynamic_cast<Bullet*>(event.bullet);
    if (!bullet) return;
    
    // Reset combo if it's a player bullet
    if (bullet->GetOwnerIdentity().IsPlayer()) {
        ResetPlayerCombo(bullet->GetOwnerIdentity().GetPlayerIndex());
    }
    
    bullet->Kill();
}

void CombatSystem::OnBulletTimeout(const BulletTimeoutEvent& event) {
    Bullet* bullet = dynamic_cast<Bullet*>(event.bullet);
    if (!bullet) return;
    
    // Reset combo if it's a player bullet
    if (bullet->GetOwnerIdentity().IsPlayer()) {
        ResetPlayerCombo(bullet->GetOwnerIdentity().GetPlayerIndex());
    }
    
    bullet->Kill();
}

void CombatSystem::HandleBulletTankCollision(Bullet* bullet, Tank* tank) {
    if (tank->identity == bullet->GetOwnerIdentity()) {
        // Self-collision (friendly fire or ricochet)
        if (bullet->GetDT() > 0.5f) {
            if (tank->health < tank->maxHealth * 2) {
                tank->health += bullet->GetPower() / 2;
            }
        }
        return;
    }
    
    if (tank->health > 0) {
        ApplyTankDamage(tank, bullet->GetPower(), bullet);
        
        // Update combos if bullet is from a player
        if (bullet->GetOwnerIdentity().IsPlayer()) {
            int playerIndex = bullet->GetOwnerIdentity().GetPlayerIndex();
            UpdatePlayerCombos(playerIndex, tank, bullet);
        }
        
        // Special bullet behavior (Blue type keeps going)
        if (bullet->GetOwnerIdentity().IsPlayer() && bullet->GetIsSpecial() && bullet->GetType1() == TankType::TYPE_BLUE) {
            // Blue special bullets don't die on hit, they get stronger
            bullet->SetAlive(true);
            bullet->SetPower(bullet->GetPower() + 100);
        }
        
        // Create collision effects
        CreateCollisionEffects(bullet->GetX(), bullet->GetY(), bullet->GetZ(), 
                             bullet->GetR(), bullet->GetG(), bullet->GetB(), tank->ry);
    }
}

void CombatSystem::HandleBulletPlayerCollision(Bullet* bullet, Tank* player) {
    if (bullet->GetOwnerIdentity() == player->identity) {
        // Self-collision
        if (bullet->GetDT() > 0.5f) {
            App::GetSingleton().soundTask->PlayChannel(3);
            if (player->health < player->maxHealth) {
                player->health += bullet->GetPower() / 2;
            } else if (player->energy < player->maxEnergy) {
                player->energy += bullet->GetPower() / 8;
            }
        }
        return;
    }
    
    if (bullet->GetOwnerIdentity().IsPlayer() && !App::GetSingleton().gameTask->IsVersusMode()) {
        // Friendly fire in non-versus mode (healing)
        App::GetSingleton().soundTask->PlayChannel(3);
        if (player->health < player->maxHealth * 2) {
            player->health += bullet->GetPower() / 2;
        } else if (player->energy < player->maxEnergy * 2) {
            player->energy += bullet->GetPower() / 8;
        }
    } else {
        // Hostile fire
        ApplyTankDamage(player, bullet->GetPower(), bullet);
        
        // Create star effects for player hits
        Events::GetBus().Post(CreateFXEvent(static_cast<int>(FxType::TYPE_STAR), 
            bullet->GetX(), bullet->GetY(), bullet->GetZ(), 0, .01f, 0, 0, player->ry, 90, 
            bullet->GetR(), bullet->GetG(), bullet->GetB(), 1));
        Events::GetBus().Post(CreateFXEvent(static_cast<int>(FxType::TYPE_STAR), 
            bullet->GetX(), bullet->GetY(), bullet->GetZ(), 0, .01f, 2, 0, bullet->GetRY(), 90, 
            bullet->GetR(), bullet->GetG(), bullet->GetB(), 1));
        
        App::GetSingleton().soundTask->PlayChannel(8);
        
        // Update attacker combos
        if (bullet->GetOwnerIdentity().IsEnemy()) {  // Enemy bullet hit player
            // Could implement enemy combo system here if needed
        }
    }
}

void CombatSystem::ApplyTankDamage(Tank* tank, float damage, Bullet* source) {
    tank->health -= damage;
    
    // Post damage event
    Events::GetBus().Post(TankDamagedEvent(tank, source, damage, tank->health, tank->health <= 0));
}

void CombatSystem::UpdatePlayerCombos(int playerIndex, Tank* target, Bullet* bullet) {
    // Safety: Validate playerIndex bounds for TankHandler arrays (size 2)
    if (playerIndex < 0 || playerIndex >= 2) {
        Logger::Get().Write("WARNING: UpdatePlayerCombos - invalid playerIndex=%d\n", playerIndex);
        return;
    }
    
    Logger::Get().Write("CombatSystem::UpdatePlayerCombos - playerIndex=%d, bulletId=%d, targetHealth=%.2f\n", 
                       playerIndex, bullet->GetTankId(), target->health);
    
    // Get player from PlayerManager
    Player* player = App::GetSingleton().gameTask->GetPlayerManager()->GetPlayer(playerIndex);
    if (!player) {
        Logger::Get().Write("WARNING: UpdatePlayerCombos - no player found for index=%d\n", playerIndex);
        return;
    }
    
    // Increment hit combo
    player->IncrementHitCombo();
    
    // Add to combo based on hit streak
    float comboBonus = static_cast<float>(player->GetHitCombo()) / 10.0f;
    player->AddCombo(comboBonus, player->GetComboNumber());
    
    if (target->health <= 0) {
        // Tank was killed - get player tank for position
        Tank* playerTank = player->GetControlledTank();
        if (!playerTank) return; // No player tank available
        
        float distx = bullet->GetX() - playerTank->x;
        float distz = bullet->GetZ() - playerTank->z;
        float dist = std::sqrt(distx * distx + distz * distz);
        
        // Combo bonuses based on conditions
        if (player->GetHitCombo() > 9) {
            player->AddCombo(player->GetComboNumber() / 2.0f, player->GetComboNumber());
            playerTank->bonus = 23;
            playerTank->bonusTime = 0;
            Events::GetBus().Post(CreateFXEvent(static_cast<int>(FxType::TYPE_SMALL_SQUARE),
                playerTank->x, playerTank->y, playerTank->z,
                0, .01f, 0, 0, 0, 90, 0.5f, 0.5f, 0, 1));
        }
        
        if (dist > 20) {
            player->AddCombo(dist / 10.0f, player->GetComboNumber());
            playerTank->bonus = 21;
            playerTank->bonusTime = 0;
            Events::GetBus().Post(CreateFXEvent(static_cast<int>(FxType::TYPE_SMALL_SQUARE),
                playerTank->x, playerTank->y, playerTank->z,
                0, .01f, 0, 0, 0, 90, 0, 1, 1, 1));
        }
        
        if (bullet->GetBounces() > 0 && bullet->GetDT() < 10.0f) {
            player->AddCombo(1 + bullet->GetDT() / 2.0f, player->GetComboNumber());
            playerTank->bonus = 22;
            playerTank->bonusTime = 0;
            Events::GetBus().Post(CreateFXEvent(static_cast<int>(FxType::TYPE_SMALL_SQUARE),
                playerTank->x, playerTank->y, playerTank->z,
                0, .01f, 0, 0, 0, 90, 0.5f, 0.5f, 0, 1));
        }
        
        // Add kill bonus
        float currentCombo = player->GetCombo();
        player->AddCombo(10.0f / (currentCombo / 10.0f + 1.0f), player->GetComboNumber());
        
        // Update special charge if combo is higher
        if (player->GetCombo() > player->GetSpecialCharge()) {
            player->SetSpecialCharge(player->GetCombo());
        }
        
        // Increment combo number (kill count)
        player->AddCombo(0, player->GetComboNumber() + 1);
    }
}

void CombatSystem::CreateCollisionEffects(float x, float y, float z, float r, float g, float b, float angle) {
    Events::GetBus().Post(CreateFXEvent(static_cast<int>(FxType::TYPE_SMALL_SQUARE), 
        x, y, z, 0, .01f, 0, 0, angle, 90, r, g, b, 1));
    Events::GetBus().Post(CreateFXEvent(static_cast<int>(FxType::TYPE_SMALL_SQUARE), 
        x, y, z, 0, .01f, 2, 0, angle + 180, 90, r, g, b, 1));
}

void CombatSystem::ResetPlayerCombo(int playerIndex) {
    if (playerIndex < 0 || playerIndex >= 4) return;
    Player* player = App::GetSingleton().gameTask->GetPlayerManager()->GetPlayer(playerIndex);
    if (player) {
        player->ResetHitCombo();
    }
}
