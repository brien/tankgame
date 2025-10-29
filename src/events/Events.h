#pragma once

#include "EventBus.h"

/**
 * Static wrapper around EventBus for global access.
 * Provides convenient static methods for event publishing and subscription.
 */
class Events {
public:
    // Get the global event bus instance
    static EventBus& GetBus() {
        static EventBus instance;
        return instance;
    }
    
    // Convenience methods that delegate to the global bus
    template<typename EventType>
    static void Subscribe(std::function<void(const EventType&)> handler) {
        GetBus().Subscribe<EventType>(handler);
    }
    
    template<typename EventType>
    static void Publish(const EventType& event) {
        GetBus().Publish<EventType>(event);
    }
    
    template<typename EventType>
    static void Post(const EventType& event) {
        GetBus().Post<EventType>(event);
    }
    
    static void ProcessQueuedEvents() {
        GetBus().ProcessQueuedEvents();
    }
    
    static void Clear() {
        GetBus().Clear();
    }

private:
    // Prevent instantiation
    Events() = default;
};
