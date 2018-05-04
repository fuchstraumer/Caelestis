#pragma once
#ifndef VPSK_SIGNALS_EMITTER_HPP
#define VPSK_SIGNALS_EMITTER_HPP
#include <algorithm>
#include <vector>
#include <memory>
#include <forward_list>

namespace vpsk {

    template<typename Derived>
    class Emitter {

        struct handlerBase {
            virtual ~handlerBase() = default;
            virtual bool empty() const noexcept = 0;
            virtual void clear() noexcept = 0;
        };

        std::vector<std::unique_ptr<handlerBase>> handlers;

        template<typename EventType>
        struct Handler final : handlerBase {
            using listener_type = std::function<void(const EventType&, Derived&)>;
            using element_type = std::pair<bool, listener_type>;
            using container_type = std::forward_list<element_type>;
            using connection_type = typename container_type::iterator;

            bool empty() const noexcept final {
                auto predicate = [](auto&& elem){ return elem.first };
                return std::all_of(singleUseListeners.cbegin(), singleUseListeners.cend(), predicate) &&
                    std::all_of(listeners.cbegin(), listeners.cend(), predicate);
            };

            void clear() noexcept final {
                if (publishing) {
                    auto set_all_published = [](auto&& elem) { elem.first = true };
                    std::for_each(singleUseListeners.begin(), singleUseListeners.end(), set_all_published);
                    std::for_each(listeners.begin(), listeners.end(), set_all_published);
                }
                else {
                    singleUseListeners.clear();
                    listeners.clear();
                }
            }

            inline connection_type addSingleUseListener(listener_type listener) {
                singleUseListeners.emplace_after(singleUseListeners.before_begin(), false, std::move(listener));
            }

            inline connection_type addListener(listener_type listener) {
                listeners.emplace_after(listeners.before_begin(), false, std::move(listener));
            }

            void erase(connection_type connection) noexcept;

            void publish(const Event& event, Derived& ref);

        private:
            bool publishing = false;
            container_type singleUseListeners;
            container_type listeners;
        };

        static std::size_t next() noexcept {
            static std::size_t counter = 0;
            return counter++;
        }

        template<typename>
        static std::size_t type() noexcept {
            static std::size_t value = next();
            return value;
        }

        template<typename Event>
        Handler<Event>& handle() noexcept {
            const std::size_t family = type<Event>();

            if (family >= handlers.size()) {
                handlers.resize(family + 1);
            }
            
            if (handlers[family] == nullptr) {
                handlers[family] = std::make_unique<Handler<Event>>();
            }

            return static_cast<Handle<Event>&>(*handlers[family]);
        }

    public:

        template<typename EventType>
        using Listener = typename Handler<EventType>::listener_type;

        template<typename EventType>
        struct Connection final : private Handler<EventType>::connection_type {
            friend class Emitter;

            Connection() noexcept = default;
            Connection(typename Handler<EventType>::connection_type connection) : Handler<EventType>::connection_type{std::move(connection)};
        };

        Emitter() noexcept = default;
        Emitter(const Emitter&) = delete;
        Emitter(Emitter&& other) noexcept : handlers(std::move(other.handlers)) {}
        Emitter& operator=(const Emitter&) = delete;
        Emitter& operator=(Emitter&& other) noexcept {
            handlers = std::move(other.handlers);
            return *this;
        }

        virtual ~Emitter() noexcept {
            static_assert(std::is_base_of_v<Emitter<Derived>,Derived>, "!");
        }

        template<typename EventType, typename...Args>
        void Publish(Args&&...args) {
            handler<EventType>().publish({std::forward<Args&&>(args)...},*static_cast<Derived*>(this));
        }

        template<typename EventType>
        Connection<EventType> ConnectListener(Listener<EventType> listener) {
            return handler<EventType>().addListener(std::move(listener));
        }

        template<typename EventType>
        Connection<EventType> ConnectSingleUseListener(Listener<EventType> listener) {
            return handler<EventType>().addSingleUseListener(std::move(listener));
        }

        template<typename EventType>
        void Disconnect(Connection<EventType> connection) noexcept {
            handler<EventType>().erase(std::move(connection));
        }

        template<typename EventType>
        void DisconnectAllOfType() noexcept {
            handler<EventType>().clear();
        }

        void DisconnectAll() noexcept {
            std::for_each(handlers.begin(), handlers.end(),
                [](auto&& handler){ 
                    if (handler) {
                        handler->clear();
                    }
                });
        }

        template<typename EventType>
        bool NoListenersForEventType() const noexcept {
            const std::size_t family = type<EventType>();
            return (!(family < handlers.size()) || !handlers[family] || static_cast<Handler<EventType>&>(*handlers[family]).empty());
        }

        bool HasAnyListeners() const noexcept {
            return std::all_of(handlers.cbegin(), handlers.cend(),
                [](auto&& handler){ return !handler || handler->empty(); });
        }
        
    };

    template<typename Derived>
    template<typename EventType>
    void Emitter<Derived>::Handler<EventType>::erase(Emitter<Derived>::Handler<EventType>::connection_type connection) noexcept {
        connection->first = true;

        if (!publishing) {
            auto predicate = [](auto&& element) { return element.first; };
            singleUseListeners.remove_if(predicate);
            listeners.remove_if(predicate);
        }
    }
    
}


#endif //!VPSK_SIGNALS_EMITTER_HPP