#include "CombatSystem.h"
#include "../events/Events.h"
#include "../Bullet.h"
#include "../Tank.h"
#include "../TankHandler.h"
#include "../FXHandler.h"
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
    if (bullet->GetTankId() < 0) {
        ResetPlayerCombo((-1 * bullet->GetTankId()) - 1);
    }
    
    bullet->Kill();
}

void CombatSystem::OnBulletTimeout(const BulletTimeoutEvent& event) {
    Bullet* bullet = dynamic_cast<Bullet*>(event.bullet);
    if (!bullet) return;
    
    // Reset combo if it's a player bullet
    if (bullet->GetTankId() < 0) {
        ResetPlayerCombo((-1 * bullet->GetTankId()) - 1);
    }
    
    bullet->Kill();
}

void CombatSystem::HandleBulletTankCollision(Bullet* bullet, Tank* tank) {
    if (tank->id == bullet->GetTankId()) {
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
        if (bullet->GetTankId() < 0) {
            int playerIndex = (-1 * bullet->GetTankId()) - 1;
            UpdatePlayerCombos(playerIndex, tank, bullet);
        }
        
        // Special bullet behavior (Blue type keeps going)
        if (bullet->GetTankId() < 0 && bullet->GetIsSpecial() && bullet->GetType1() == TankType::TYPE_BLUE) {
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
    if (bullet->GetTankId() == player->id) {
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
    
    if (bullet->GetTankId() < 0 && !App::GetSingleton().gameTask->IsVersusMode()) {
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
        if (bullet->GetTankId() >= 0) {  // Enemy bullet hit player
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
    if (playerIndex < 0 || playerIndex >= 4 || !playerManager) return;
    
    playerManager->AddHitCombo(playerIndex, 1);
    playerManager->AddCombo(playerIndex, static_cast<float>(playerManager->GetHitCombo(playerIndex)) / 10.0f);
    
    if (target->health <= 0) {
        // Tank was killed
        float distx = bullet->GetX() - playerManager->GetPlayer(playerIndex).x;
        float distz = bullet->GetZ() - playerManager->GetPlayer(playerIndex).z;
        float dist = std::sqrt(distx * distx + distz * distz);
        
        // Combo bonuses based on conditions
        if (playerManager->GetHitCombo(playerIndex) > 9) {
            playerManager->AddCombo(playerIndex, playerManager->GetComboNum(playerIndex) / 2.0f);
            playerManager->GetPlayer(playerIndex).bonus = 23;
            playerManager->GetPlayer(playerIndex).bonusTime = 0;
            Events::GetBus().Post(CreateFXEvent(static_cast<int>(FxType::TYPE_SMALL_SQUARE),
                playerManager->GetPlayer(playerIndex).x, playerManager->GetPlayer(playerIndex).y, playerManager->GetPlayer(playerIndex).z,
                0, .01f, 0, 0, 0, 90, 0.5f, 0.5f, 0, 1));
        }
        
        if (dist > 20) {
            playerManager->AddCombo(playerIndex, dist / 10.0f);
            playerManager->GetPlayer(playerIndex).bonus = 21;
            playerManager->GetPlayer(playerIndex).bonusTime = 0;
            Events::GetBus().Post(CreateFXEvent(static_cast<int>(FxType::TYPE_SMALL_SQUARE),
                playerManager->GetPlayer(playerIndex).x, playerManager->GetPlayer(playerIndex).y, playerManager->GetPlayer(playerIndex).z,
                0, .01f, 0, 0, 0, 90, 0, 1, 1, 1));
        }
        
        if (bullet->GetBounces() > 0 && bullet->GetDT() < 10.0f) {
            playerManager->AddCombo(playerIndex, (1 + bullet->GetDT() / 2.0f));
            playerManager->GetPlayer(playerIndex).bonus = 22;
            playerManager->GetPlayer(playerIndex).bonusTime = 0;
            Events::GetBus().Post(CreateFXEvent(static_cast<int>(FxType::TYPE_SMALL_SQUARE),
                playerManager->GetPlayer(playerIndex).x, playerManager->GetPlayer(playerIndex).y, playerManager->GetPlayer(playerIndex).z,
                0, .01f, 0, 0, 0, 90, 0.5f, 0.5f, 0, 1));
        }
        
        float currentCombo = playerManager->GetCombo(playerIndex);
        playerManager->AddCombo(playerIndex, (10.0f / (currentCombo / 10.0f + 1.0f)));
        
        if (playerManager->GetCombo(playerIndex) > playerManager->GetSpecial(playerIndex)) {
            playerManager->SetSpecial(playerIndex, playerManager->GetCombo(playerIndex));
        }
        
        playerManager->AddComboNum(playerIndex, 1);
    }
}

void CombatSystem::CreateCollisionEffects(float x, float y, float z, float r, float g, float b, float angle) {
    Events::GetBus().Post(CreateFXEvent(static_cast<int>(FxType::TYPE_SMALL_SQUARE), 
        x, y, z, 0, .01f, 0, 0, angle, 90, r, g, b, 1));
    Events::GetBus().Post(CreateFXEvent(static_cast<int>(FxType::TYPE_SMALL_SQUARE), 
        x, y, z, 0, .01f, 2, 0, angle + 180, 90, r, g, b, 1));
}

void CombatSystem::ResetPlayerCombo(int playerIndex) {
    if (playerIndex < 0 || playerIndex >= 4 || !playerManager) return;
    playerManager->ResetHitCombo(playerIndex);
}
