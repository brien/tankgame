#pragma once

#include <functional>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include "Event.h"

/**
 * Central event bus for decoupled communication between game systems.
 * Systems can publish events and subscribe to events they care about.
 */
class EventBus {
public:
    // Type-erased event handler
    using EventHandler = std::function<void(const Event&)>;
    
    // Subscribe to events of a specific type
    template<typename EventType>
    void Subscribe(std::function<void(const EventType&)> handler) {
        auto typeIndex = std::type_index(typeid(EventType));
        
        // Wrap the typed handler in a type-erased handler
        auto wrappedHandler = [handler](const Event& event) {
            handler(static_cast<const EventType&>(event));
        };
        
        handlers[typeIndex].push_back(wrappedHandler);
    }
    
    // Publish an event immediately (synchronous)
    template<typename EventType>
    void Publish(const EventType& event) {
        auto typeIndex = std::type_index(typeid(EventType));
        auto it = handlers.find(typeIndex);
        
        if (it != handlers.end()) {
            for (auto& handler : it->second) {
                handler(event);
            }
        }
    }
    
    // Post an event to be processed later (asynchronous)
    template<typename EventType>
    void Post(const EventType& event) {
        eventQueue.push_back(std::make_unique<EventType>(event));
    }
    
    // Process all queued events
    void ProcessQueuedEvents() {
        auto currentQueue = std::move(eventQueue);
        eventQueue.clear();
        
        for (auto& event : currentQueue) {
            auto typeIndex = std::type_index(event->GetType());
            auto it = handlers.find(typeIndex);
            
            if (it != handlers.end()) {
                for (auto& handler : it->second) {
                    handler(*event);
                }
            }
        }
    }
    
    // Clear queued events only (keeps subscriptions intact)
    void Clear() {
        eventQueue.clear();
    }
    
    // Clear all handlers and queued events (use sparingly!)
    void ClearAll() {
        handlers.clear();
        eventQueue.clear();
    }

private:
    std::unordered_map<std::type_index, std::vector<EventHandler>> handlers;
    std::vector<std::unique_ptr<Event>> eventQueue;
};
