#pragma once

#include <algorithm>
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
    void Subscribe(std::function<void(const EventType&)> handler, const void* owner = nullptr) {
        auto typeIndex = std::type_index(typeid(EventType));
        
        // Wrap the typed handler in a type-erased handler
        auto wrappedHandler = [handler](const Event& event) {
            handler(static_cast<const EventType&>(event));
        };
        
        handlers[typeIndex].push_back(std::make_shared<Subscription>(Subscription{wrappedHandler, owner, true}));
    }
    
    // Remove only this owner's callbacks. Unowned subscriptions remain until ClearAll().
    // Owners must unsubscribe before destruction and keep a stable address while subscribed.
    void Unsubscribe(const void* owner) {
        if (!owner) return;
        for (auto it = handlers.begin(); it != handlers.end();) {
            auto& subscriptions = it->second;
            subscriptions.erase(std::remove_if(subscriptions.begin(), subscriptions.end(),
                [owner](const std::shared_ptr<Subscription>& subscription) {
                    if (subscription->owner != owner) return false;
                    subscription->active = false;
                    return true;
                }), subscriptions.end());
            if (subscriptions.empty()) {
                it = handlers.erase(it);
            } else {
                ++it;
            }
        }
    }

    // Publish an event immediately (synchronous)
    template<typename EventType>
    void Publish(const EventType& event) {
        Dispatch(std::type_index(typeid(EventType)), event);
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
            Dispatch(std::type_index(event->GetType()), *event);
        }
    }
    
    // Clear queued events only (keeps subscriptions intact)
    void Clear() {
        eventQueue.clear();
    }
    
    // Clear all handlers and queued events (use sparingly!)
    void ClearAll() {
        for (auto& entry : handlers) {
            for (auto& subscription : entry.second) {
                subscription->active = false;
            }
        }
        handlers.clear();
        eventQueue.clear();
    }

private:
    struct Subscription {
        EventHandler handler;
        const void* owner;
        bool active;
    };

    void Dispatch(std::type_index type, const Event& event) {
        auto it = handlers.find(type);
        if (it == handlers.end()) return;

        // A callback may subscribe, unsubscribe, or destroy another subscriber.
        // Keep callbacks alive until dispatch ends, but skip any removed callbacks.
        // New subscriptions start with the next dispatch (including nested publishes).
        auto snapshot = it->second;
        for (const auto& subscription : snapshot) {
            if (subscription->active) {
                subscription->handler(event);
            }
        }
    }

    std::unordered_map<std::type_index, std::vector<std::shared_ptr<Subscription>>> handlers;
    std::vector<std::unique_ptr<Event>> eventQueue;
};
