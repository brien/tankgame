#pragma once

#include <vector>
#include <memory>
#include <algorithm>

/**
 * Generic entity manager template.
 * Handles lifecycle management for any entity type.
 */
template<typename T>
class EntityManager {
private:
    std::vector<std::unique_ptr<T>> entities;
    
public:
    /**
     * Update all entities and remove dead ones.
     * This replaces the scattered update loops in handlers.
     */
    void Update() {
        // Update living entities
        for (auto& entity : entities) {
            if (entity->IsAlive()) {
                entity->Update();
            }
        }
        
        // Remove dead entities (same pattern as existing handlers)
        entities.erase(
            std::remove_if(entities.begin(), entities.end(),
                [](const auto& entity) {
                    if (!entity->IsAlive()) {
                        entity->OnDestroy();  // Call cleanup
                        return true;
                    }
                    return false;
                }),
            entities.end()
        );
    }
    
    /**
     * Create a new entity with given parameters.
     * Returns raw pointer for immediate use.
     */
    template<typename... Args>
    T* Create(Args&&... args) {
        auto entity = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = entity.get();
        entities.push_back(std::move(entity));
        return ptr;
    }
    
    /**
     * Get all entities (read-only access).
     */
    const std::vector<std::unique_ptr<T>>& GetEntities() const {
        return entities;
    }
    
    /**
     * Get count of living entities.
     */
    size_t GetAliveCount() const {
        return std::count_if(entities.begin(), entities.end(),
            [](const auto& entity) { return entity->IsAlive(); });
    }
    
    /**
     * Get count of all entities (including dead ones awaiting cleanup).
     */
    size_t GetTotalCount() const {
        return entities.size();
    }
    
    /**
     * Remove all entities.
     */
    void Clear() {
        entities.clear();
    }
    
    /**
     * Check if any entities exist.
     */
    bool IsEmpty() const {
        return entities.empty();
    }
};
