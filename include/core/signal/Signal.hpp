#pragma once
#ifndef VPSK_SIGNAL_HPP
#define VPSK_SIGNAL_HPP
#include <utility>
#include <vector>
#include <algorithm>

namespace vpsk {

    namespace detail {
        
        template<typename,typename>
        struct Invoker;

        template<typename Result, typename...Args, typename Collector>
        struct Invoker<Result(Args...), Collector> {
            using func_proto_type = Result(*)(void *, Args...);
            using func_call_type = std::pair<void*, func_proto_type>;
            virtual ~Invoker() noexcept = default;

            bool invoke(Collector& collector, func_proto_type function_prototype, void* instance, Args...args) {
                return collector(function_prototype(instance, args...));
            }
        };

        template<typename...Args, typename Collector>
        struct Invoker<void(Args...), Collector> {
            using func_proto_type = void(*)(void *, Args...);
            using func_call_type = std::pair<void*, func_proto_type>;
            virtual ~Invoker() noexcept = default;

            bool invoke(Collector& collector, func_proto_type function_prototype, void* instances, Args...args) {
                return (proto(instance, args...), true);
            }
        };

        template<typename Result>
        struct NullCollector final {
            using result_type = Result;
            bool operator()(result_type) const noexcept { 
                return true;
            }
        };

        template<>
        struct NullCollector<void> final {
            using result_type = void;
            bool operator()() const noexcept {
                return true;
            }
        };

        template<typename>
        struct DefaultCollector;

        template<typename Result, typename...Args>
        struct DefaultCollector<Result(Args...)> final {
            using collector_type = NullCollector<Result>;
        };

        template<typename Function>
        using DefaultCollectorType = typename DefaultCollector<Function>::collector_type;

    }

    template<typename Function>
    class Sink;

    template<typename Function, typename Collector = detail::DefaultCollectorType<Function>>
    class SignalHandler;

    // Sinks are used to connect listeners to signals: the function type for a listener is the same
    // as the signal it belongs to. 
    template<typename Result, typename...Args>
    class Sink<Result(Args...)> final {
        template<typename,typename>
        friend class SignalHandler;

        using function_prototype = Result(*)(void *, Args...);
        using function_call_type = std::pair<void*, function_prototype>;

        template<Result(*Function)(Args...)>
        static Result prototype(void*,Args...args) {
            return (Function)(args...);
        }

        template<typename Class, Result(Class::*Member)(Args...args)>
        static Result prototype(void* instance, Args...args) {
            return (static_cast<Class*>(instance)->*Member)(args...);
        }

        Sink(std::vector<function_call_type>& calls) : function_calls(calls) {}

    public:

        template<Result(*Fn)(Args...)>
        void Connect() {
            Disconnect<Fn>();
            function_calls.emplace_back(nullptr,&prototype<Fn>);
        }

        template<Result(*Fn)(Args...)>
        void Disconnect() {
            function_call_type target{nullptr,&prototype<Fn>};
            function_calls.erase(std::remove(function_calls.begin(), function_calls.end(), std::move(target)), function_calls.end());
        }

        void DisconnectAll() {
            function_calls.clear();
        }

    private:
        std::vector<function_call_type>& function_calls;
    };

    template<typename Result, typename...Args, typename Collector>
    class SignalHandler<Result(Args...), Collector> final : private detail::Invoker<Result(Args...), Collector> {
        using call_type = typename detail::Invoker<Result(Args...), Collector>::func_call_type;
    public:
        using size_type = typename std::vector<call_type>::size_type;
        using collector_type = Collector;
        using sink_type = Sink<Result(Args...)>;

        template<typename Class>
        using class_instance_type = Class*;

        size_type NumListeners() const noexcept {
            return fnCalls.size();
        }

        bool HasNoListeners() const noexcept {
            return fnCalls.empty();
        }

        sink_type GetSink() {
            return { fnCalls };
        }

        void TriggerSignal(Args...args) {
            std::for_each(fnCalls.begin(), fnCalls.end(),
            [&args...](auto&& call){ 
                call.second(call.first, args...);
            });
        }

        collector_type CollectReturnValues(Args&&...args) {
            collector_type collector;
            for (auto&& call : fnCalls) {
                if (!this->invoke(collector, call.second, call.first, args...)) {
                    break;
                }
            }

            return collector;
        }

        friend void swap(SignalHandler& lhs, SignalHandler& rhs) {
            using std::swap;
            swap(lhs.fnCalls, rhs.fnCalls);
        }

        bool operator==(const SignalHandler& other) const noexcept {
            return std::equal(fnCalls.cbegin(), fnCalls.cend(), other.fnCalls.cbegin(), other.fnCalls.cend());
        }

    private:
        std::vector<call_type> fnCalls;
    };

    template<typename Result, typename...Args>
    bool operator!=(const SignalHandler<Result(Args...)>& lhs, const SignalHandler<Result(Args...)>& rhs) noexcept {
        return !(lhs == rhs);
    }

}

#endif //!VPSK_SIGNAL_HPP
