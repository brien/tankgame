#pragma once

#include <typeinfo>
#include <memory>

/**
 * Base class for all events in the system.
 * Events are immutable data containers that describe what happened.
 */
class Event {
public:
    virtual ~Event() = default;
    virtual const std::type_info& GetType() const = 0;
};

/**
 * Template base class for specific event types.
 * Automatically implements GetType() for derived classes.
 */
template<typename T>
class EventBase : public Event {
public:
    const std::type_info& GetType() const override {
        return typeid(T);
    }
};
