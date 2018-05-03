#pragma once
#ifndef VPSK_EVENT_HPP
#define VPSK_EVENT_HPP
#include "Signal.hpp"
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

    using EventSignal = Signal<void(const void*)>;
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

    class BaseReceiver {
    public:
        virtual ~BaseReceiver() {
            for (auto& connection : connections) {
                auto& ptr = connection.second.first;
                if (!ptr.expired()) {
                    ptr.lock()->Disconnect(connection.second.second);
                }
            }
        }

        size_t ConnectedSignals() const noexcept {
            size_t result = 0;
            for (auto& connection : connections) {
                if (!connection.second.first.expired()) {
                    ++result;
                }
            }
            return result;
        }

    private:
        friend class EventManager;
        std::unordered_map<BaseEvent::Family, std::pair<EventSignalWeakPtr, size_t>> connections;
    };

    template<typename Derived>
    class Receiver : public BaseReceiver {
    public:
        virtual ~Receiver() {}
    };

    class EventManager {
        EventManager(const EventManager&) = delete;
        EventManager& operator=(const EventManager&) = delete;
    public:
        EventManager() = default;
        virtual ~EventManager() = default;

        /**Subscribe given object to events of template type E.
        */
        template<typename EventType, typename ReceiverType>
        void Subscribe(ReceiverType& receiver) {
            auto receive_ptr = &ReceiverType::Receive;
            auto signal = signalFor(Event<EventType>::Family());
            auto wrapper = event_callback_wrapper_t<EventType>(std::bind(receive_ptr, &receiver, std::placeholders::_1));
            auto connection = signal->Connect(wrapper);
            dynamic_cast<BaseReceiver&>(receiver).connections.emplace(Event<EventType>::Family(), std::make_pair(EventSignalWeakPtr(signal), connection));
        }

        template<typename EventType, typename ReceiverType>
        void Unsubscribe(ReceiverType& receiver) {
            BaseReceiver& base = dynamic_cast<BaseReceiver&>(receiver);
            auto pair = base.connections[Event<EventType>::Family()];
            auto connection = pair.second;
            auto& ptr = pair.first;
            if (!ptr.expired()) {
                ptr.lock()->Disconnect(connection);
            }
            base.connections.erase(Event<EventType>::Family());
        }

        template<typename EventType>
        void Emit(const EventType& event) {
            auto signal = signalFor(Event<EventType>::Family());
            signal->Emit(event);
        }

        template<typename EventType>
        void Emit(const std::unique_ptr<EventType>& event) {
            auto signal = signalFor(Event<EventType>::Family());
            signal->Emit(event.get());
        }

        template<typename EventType, typename...Args>
        void Emit(Args&&...args) {
            EventType event(std::forward<Args&&>(args)...);
            auto signal = signalFor(Event<EventType>::Family());
            signal->Emit(&event);
        }

        size_t ConnectedReceivers() const noexcept {
            size_t result = 0;
            for (auto handle : handlers) {
                if (handle) {
                    result += handle->size();
                }
            }
            return result;
        }

    private:

        EventSignalPtr & signalFor(const size_t id) {
            if (id >= handlers.size()) {
                handlers.resize(id + 1);
            }
            if (!handlers[id]) {
                handlers[id] = std::make_shared<EventSignal>();
            }
            return handlers[id];
        }

        template<typename EventType>
        struct event_callback_wrapper_t {
            explicit event_callback_wrapper_t(std::function<void(const EventType& event)> _callback) : callback(std::move(_callback)) {}
            void operator()(const void* event) {
                callback(*(static_cast<const EventType*>(event)));
            }
            std::function<void(const EventType& event)> callback;
        };

        std::vector<EventSignalPtr> handlers;
    };
}

#endif // !VPSK_EVENT_HPP
