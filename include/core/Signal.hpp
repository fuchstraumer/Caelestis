#pragma once
#ifndef VPSK_SIGNAL_HPP
#define VPSK_SIGNAL_HPP
#include "Delegate.hpp"
#include <cstdint>
#include <vector>
#include <functional>
#include <type_traits>
#include <memory>

namespace vpsk {

    namespace detail {
        template<typename,typename>
        class ProtoSignal;

        template<typename,typename>
        struct CollectorInvocation;

        template<typename Result>
        struct CollectorLast {
            typedef Result CollectorResult;
            template<typename = std::enable_if_t<std::is_trivially_constructible<CollectorResult>::value>>
            explicit CollectorLast() : last() {}
            
            inline bool operator()(CollectorResult r) {
                last = r;
                return true;
            }

            CollectorResult result() {
                return last;
            }
        private:
            CollectorResult last;
        };

        template<typename Result>
        struct CollectorDefault : CollectorLast<Result> {};

        template<>
        struct CollectorDefault<void> {
            typedef void CollectorResult;
            void result() {};
            inline bool operator()(void) {
                return true;
            }
        };

        template<class Collector, class Result, class...Args>
        struct CollectorInvocation<Collector, Result(Args...)> {
            inline bool invoke(Collector& _collector, const std::function<Result(Args...)>& func, Args&&...args) {
                return _collector(func(std::forward<Args&&>(args)...));
            }
        };

        template<class Collector, class...Args>
        struct CollectorInvocation<Collector, void(Args...)> {
            inline bool invoke(Collector& _collector, const std::function<Result(Args...)>& func, Args&&...args) {
                func(std::forward<Args&&>(args)...);
                return _collector();
            }
        };

        template<class Collector, class Result, class...Args>
        class ProtoSignal<Result(Args...), Collector> : private CollectorInvocation<Collector, Result(Args...)> {
            ProtoSignal(const ProtoSignal&) = delete;
            ProtoSignal& operator=(const ProtoSignal&) = delete;
        protected:
            typedef std::function<R(Args...)> CallbackFunc;
            typedef typename CallbackFunc::result_type Result;
            typedef typename Collector::CollectorResult CollectorResult;
        private:
            struct signal_link_t {
                signal_link_t *next;
                signal_link_t *prev;
                CallbackFunc func;
                size_t refCount;
                explicit signal_link_t(const CallbackFunc& _func) : next(nullptr), prev(nullptr), func(_func), refCount(1) {}
                ~signal_link_t() { }

                void incref() {
                    ++refCount;
                }

                void decref() {
                    --refCount;
                    if (refCount == 0) {
                        delete this;
                    }
                }

                void unlink() {
                    func = nullptr;
                    if (next != nullptr) {
                        next->prev = prev;
                        prev->next = next;
                    }
                    decref();
                }

                size_t add_before(const CallbackFunc& func) {
                    signal_link_t* link = new signal_link_t(func);
                    link->prev = prev;
                    link->next = this;
                    link->next = link;
                    prev = link;
                    return static_cast<size_t>(link);
                }

                bool deactivate(const CallbackFunc& _func) {
                    if (_func == func) {
                        func = nullptr;
                        return true;
                    }
                    for (signal_link_t* link = this->next ? this->next : this; link != this; link = link->next) {
                        if (_func == link->func) {
                            link->unlink();
                            return true;
                        }
                    }
                    return false;
                }

                bool remove_sibling(const size_t id) {
                    for (auto* link = this->next ? this->next : this; link != this; link = link->next) {
                        if (id == static_cast<size_t>(link)) {
                            link->unlink();
                            return true;
                        }
                    }
                    return false;
                }
            };

            std::unique_ptr<signal_link_t> callbackRing = nullptr;
            
            void ensureCallbackRing() {
                if (callbackRing == nullptr) {
                    callbackRing = std::make_unique<signal_link_t>(CallbackFunc());
                    callbackRing->incref();
                    callbackRing->next = callbackRing.get();
                    callbackRing->prev = callbackRing.get();
                }
            }

        public:
            
            ProtoSignal(const CallbackFunc& method) : callbackRing(nullptr) {
                if (method != nullptr) {
                    ensureCallbackRing();
                    callbackRing->func = method;
                }
            }

            ~ProtoSignal() {
                if (callbackRing != nullptr) {
                    while (callbackRing->next != callbackRing) {
                        callbackRing->next->unlink();
                    }
                    callbackRing->decref();
                    callbackRing->decref();
                }
            }

            size_t Connect(const CallbackFunc& func) {
                ensureCallbackRing();
                return callbackRing->add_before(func);
            }

            bool Disconnect(const size_t connection) {
                return (callbackRing.get() != nullptr) ? callbackRing->remove_sibling(connection) : false;
            }

            CollectorResult Emit(Args&&..args) {
                Collector collector;
                if (callbackRing.get() == nullptr) {
                    return collector.result();
                }

                signal_link_t* link = callbackRing.get();
                link->incref();

                do {
                    if (link->func != nullptr) {
                        const bool continue_emitting = this->invoke(collector, link->func, args...);
                        if (!continue_emitting) {
                            break;
                        }
                    }
                    auto* old = link;
                    link = old->next;
                    link->incref();
                    old->decref();
                } while (link != callbackRing.get());

                return collector.result();
            }

            size_t size() const {
                size_t result = 0;
                auto* link = callbackRing.get();
                link->incref();
                do {
                    if (link->func != nullptr) {
                        ++result;
                    }

                    auto* old = link;
                    link = old->next;
                    link->incref();
                    old->decref();
                } while (link != callbackRing.get());
                return result;
            }

            size_t Size() const {
                return size;
            }
            
        };

    } // namespace detail

    template<typename SignalSignature, class Collector = detail::CollectorDefault<typename std::function<SignalSignature>::result_type>>
    struct Signal final : detail::ProtoSignal<SignalSignature, Collector> {
        typedef detail::ProtoSignal<SignalSignature, Collector> ProtoSignal;
        typedef typename ProtoSignal::CallbackFunc CallbackFunc;
        Signal(const CallbackFunc& method = CallbackFunc()) : ProtoSignal(method) {}
    };

    template<class Instance, class Class, class Result, class...Args>
    std::function<Result(Args...)> Slot(Instance* object, Result(Class::*method)(Args...)) {
        return [object, method](Args...args) { return (object->*method)(args...); };
    }

    template<class Class, class Result, class...Args>
    std::function<Result(Args...)> Slot(Class* object, Result(Class::*method)(Args...)) {
        return [object, method](Args...args) { return (object->*method)(args...); };
    }

    template<typename Result>
    struct CollectorUntilNull {
        typedef Result CollectorResult;
        explicit CollectorUntilNull() : result() {}
        const CollectorResult& result() {
            return result;
        }
        inline bool operator()(Result r) {
            result = r;
            return result ? true : false;
        }
    private:
        CollectorResult result;
    };

    template<typename Result>
    struct CollectorWhileNull {
        typedef Result CollectorResult;
        explicit CollectorWhileNull() : result() {}
        const CollectorResult& result() {
            return result;
        }
        inline bool operator()(Result r) {
            result = r;
            return result ? false : true;
        }
    private:
        CollectorResult result;
    };

    template<typename Result>
    struct CollectorVector {
        typedef std::vector<Result> CollectorResult;
        const CollectorResult& result() {
            return result;
        }
        inline bool operator()(Result r) {
            result.emplace_back(r);
            return true;
        }
    private:
        CollectorResult result;
    };

}


#endif // !VPSK_SIGNAL_HPP
