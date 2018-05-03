#pragma once
#ifndef VPSK_EVENT_HPP
#define VPSK_EVENT_HPP
#include "Delegate.hpp"
#include <utility>
#include <unordered_map>
#include <list>
#include <memory>

namespace vpsk {

    class EventManager;

    class BaseEvent {
    public:
        using Family = size_t;
        virtual ~BaseEvent() {};
    protected:
        static Family& FamilyCounter() {
            static Family counter = 0;
            return counter;
        }
    };

    using EventSignal = delegate_t<void(const void*)>;
    using EventSignalPtr = std::shared_ptr<EventSignal>;
    using EventSignalWeakPtr = std::weak_ptr<EventSignal>;

    template<typename Derived>
    class Event : public BaseEvent {
    public:
        static Family Family() {
            static Family our_family = FamilyCounter()++;
            return our_family;
        }
    };

    
}

#endif // !VPSK_EVENT_HPP
