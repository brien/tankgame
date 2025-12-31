#pragma once

// Tank identity - distinguishes player tanks from enemy tanks
struct TankIdentity {
    enum class Owner {
        PLAYER,
        ENEMY
    };
    
    Owner owner;
    int index;  // Player index (0, 1) or enemy index (0, 1, 2...)
    
    // Factory methods for clarity
    static TankIdentity Player(int playerIndex) {
        return TankIdentity{Owner::PLAYER, playerIndex};
    }
    
    static TankIdentity Enemy(int enemyIndex) {
        return TankIdentity{Owner::ENEMY, enemyIndex};
    }
    
    // Query methods
    bool IsPlayer() const { return owner == Owner::PLAYER; }
    bool IsEnemy() const { return owner == Owner::ENEMY; }
    
    int GetPlayerIndex() const { 
        return IsPlayer() ? index : -1;
    }
    
    int GetEnemyIndex() const {
        return IsEnemy() ? index : -1;
    }
    
    // Legacy compatibility - converts to old negative ID system
    // Player 0 -> -1, Player 1 -> -2
    // Enemy 0 -> 0, Enemy 1 -> 1, etc.
    int GetLegacyId() const {
        if (IsPlayer()) {
            return -(index + 1);
        } else {
            return index;
        }
    }
    
    // Equality comparison
    bool operator==(const TankIdentity& other) const {
        return owner == other.owner && index == other.index;
    }
    
    bool operator!=(const TankIdentity& other) const {
        return !(*this == other);
    }
};
