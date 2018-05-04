#pragma once
#ifndef VPSK_EVENT_DISPATCHER_HPP
#define VPSK_EVENT_DISPATCHER_HPP
#include <vector>
#include <array>
#include <memory>
#include <utility>
#include <algorithm>
#include "Signal.hpp"
#include "../Family.hpp"

namespace vpsk {

    class Dispatcher final {

        using event_family = Family<struct InternalDispatcherEventFamily>;

        template<typename Class, typename EventType>
        using instance_type = typename SignalHandler<void(const EventType&)>::template instance_type<Class>;

        struct BaseSignalWrapper { 
            virtual ~BaseSignalWrapper() noexcept = default;
            virtual void PublishSignal() = 0;
        };

        template<typename EventType>
        struct SignalWrapper final : BaseSignalWrapper {
            using sink_type = typename SignalHandler<void(const EventType&)>::sink_type;

            void PublishSignal() final {
                const auto& current = current++;
                current %= std::extent<decltype(events)>::value;
                std::for_each(events[current].cbegin(), events[current].cend(), [this](const auto& event) { signal.TriggerSignal(event); });
                events[current].clear();
            }

            inline sink_type Sink() noexcept {
                return signal.Sink();
            }

            template<typename...Args>
            inline void Trigger(Args&&...args) {
                signal.TriggerSignal({ std::forward<Args&&>(args)... });
            }

            template<typename...Args>
            inline void Enqueue(Args&&...args) {
                events[current].emplace_back({ std::forward<Args&&>(args)... });
            }

        private:
            SignalHandler<void(const EventType&)> signal{};
            std::array<std::vector<EventType>, 2> events;
            size_t current{};
        };

        template<typename EventType>
        SignalWrapper<EventType>& wrapper() {
            const auto type = event_family::getID<EventType>();
            if (!(type < wrappers.size())) {
                wrappers.resize(type + 1);
            }

            if (!wrappers[type]) {
                wrappers[type] = std::make_unique<SignalWrapper<EventType>>();
            }

            return static_cast<SignalWrapper<EventType>&>(*wrappers[type]);
        }

    public:

        template<typename EventType>
        using sink_type = typename SignalWrapper<EventType>::sink_type;

        template<typename EventType>
        inline sink_type<EventType> Sink() noexcept {
            return wrapper<EventType>().Sink();
        }

        template<typename EventType, typename...Args>
        inline void Trigger(Args&&...args) {
            wrapper<EventType>().Trigger(std::forward<Args&&>(args)...);
        }

        template<typename EventType, typename...Args>
        inline void Enqueue(Args&&...args) {
            wrapper<EventType>().Enqueue(std::forward<Args&&>(args)...);
        }

        template<typename EventType>
        inline void PublishAllPendingOfType() {
            wrapper<EventType>().PublishSignal();
        }

        inline void PublishAllPending() {
            std::for_each(wrappers.begin(), wrappers.end(), [](auto&& wrapper) {
                return wrapper ? wrapper->PublishSignal() : void();
            });
        }


    private:
        std::vector<std::unique_ptr<BaseSignalWrapper>> wrappers;
    };

}

#endif //!VPSK_EVENT_DISPATCHER_HPP