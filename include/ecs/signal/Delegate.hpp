#pragma once
#ifndef VPSK_DELEGATE_HPP
#define VPSK_DELEGATE_HPP
#include <functional>
#include <list>
#include <forward_list>
#include <memory>

namespace vpsk {

    template<typename T>
    class base_delegate_t;

    template<typename Result, typename...Args>
    class base_delegate_t<Result(Args...)> {
    protected:
        using func_stub_t = Result(*)(void* this_ptr, Args...);

        struct invocation_element_t {
            invocation_element_t() = default;
            invocation_element_t(void* this_ptr, func_stub_t _stub) : object(this_ptr), stub(_stub) {}

            invocation_element_t(invocation_element_t&& other) noexcept : object(std::move(other.object)), stub(std::move(other.stub)) {
                other.stub = nullptr;
                other.object = nullptr;
            }

            invocation_element_t& operator=(invocation_element_t&& other) noexcept {
                object = std::move(other.object);
                other.object = nullptr;
                stub = std::move(other.stub);
                other.stub = nullptr;
                return *this;
            }

            void copy_to(invocation_element_t& destination) const noexcept {
                destination.stub = stub;
                destination.object = object;
            }

            bool operator==(const invocation_element_t& other) const noexcept {
                return (other.stub == stub) && (other.object == object);
            }

            bool operator!=(const invocation_element_t& other) const noexcept {
                return (other.stub != stub) || (other.object != object);
            }

            void* object = nullptr;
            func_stub_t stub = nullptr;
        };
    };

    template<typename T>
    class delegate_t;

    template<typename T>
    class multicast_delegate_t;

    template<typename Result, typename...Args>
    class delegate_t<Result(Args...)> final : private base_delegate_t<Result(Args...)> {
    private:
        friend class multicast_delegate_t<Result(Args...)>;
        typename base_delegate_t<Result(Args...)>::invocation_element_t invocation;
    public:
        
        delegate_t() = default;
   
        bool IsEmpty() const noexcept {
            return invocation.stub == nullptr;
        }

        bool operator==(const void* ptr) const noexcept {
            return (ptr == nullptr) && (IsEmpty());
        }

        bool operator!=(const void* ptr) const noexcept {
            return (ptr != nullptr) || (!IsEmpty());
        }

        delegate_t(const delegate_t& other) noexcept {
            other.invocation.copy_to(invocation);
        }

        delegate_t& operator=(const delegate_t& other) noexcept {
            other.invocation.copy_to(invocation);
            return *this;
        }

        //delegate_t(delegate_t&& other) noexcept : invocation(std::move(other.invocation)) {}

        //delegate_t& operator=(delegate_t&& other) noexcept {
        //    invocation = std::move(other.invocation);
        //}

        template<typename LambdaFunc>
        explicit delegate_t(const LambdaFunc& func) {
            assign((void*)(&func), lambda_stub<LambdaFunc>);
        }

        template<typename LambdaFunc>
        delegate_t& operator=(const LambdaFunc& func) {
            assign((void*)(&func), lambda_stub<LambdaFunc>);
            return *this;
        }

        // i.e like if (!ptr_object), returns true when object doesn't "exist" and false
        // when it exists. thus if (!my_delegate_t) == if (my_delegate_t.invocation.stub == nullptr)
        bool operator!() const noexcept {
            return invocation.stub == nullptr;
        }

        bool operator==(const delegate_t& other) const noexcept {
            return invocation == other.invocation;
        }

        bool operator!=(const delegate_t& other) const noexcept {
            return invocation != other.invocation;
        }

        bool operator==(const multicast_delegate_t<Result(Args...)>& other) const noexcept {
            return other == (*this);
        }

        bool operator!=(const multicast_delegate_t<Result(Args...)>& other) const noexcept {
            return other != (*this);
        }

        Result operator()(Args...args) const {
            return (*invocation.stub)(invocation.object,std::forward<Args>(args)...);
        }

        template<class T, Result(T::*Method)(Args...)>
        static delegate_t create(T* object) {
            return delegate_t{ object, method_stub<T,Method> };
        }

        template<class T, Result(T::*Method)(Args...) const>
        static delegate_t create(const T* object) {
            return delegate_t{ const_cast<T*>(object), const_method_stub<T,Method> };
        }

        template<Result(*Function)(Args...)>
        static delegate_t create() {
            return delegate_t(nullptr, function_stub<Function>);
        }

        template<typename LambdaFunc>
        static delegate_t create(const LambdaFunc& lambda) {
            return delegate_t((void*)(&lambda), lambda_stub<LambdaFunc>);
        }

    private:

        explicit delegate_t(void* obj, typename base_delegate_t<Result(Args...)>::func_stub_t stub) {
            this->invocation.object = obj;
            this->invocation.stub = stub;
        }

        void assign(void* object_ptr, typename base_delegate_t<Result(Args...)>::func_stub_t _stub) {
            this->invocation.object = object_ptr;
            this->invocation.stub = _stub;
        }

        template<class T, Result(T::*Method)(Args...)>
        static Result method_stub(void* this_ptr, Args...args) {
            T* object_ptr = static_cast<T*>(this_ptr);
            return (object_ptr->*Method)(args...);
        }

        template<class T, Result(T::*Method)(Args...) const>
        static Result const_method_stub(void* this_ptr, Args...args) {
            T* const object_ptr = static_cast<T*>(this_ptr);
            return (object_ptr->*Method)(args...);
        }

        template<Result(*Function)(Args...)>
        static Result function_stub(void* this_ptr, Args...args) {
            return (Function)(args...);
        }

        template<typename LambdaFunc>
        static Result lambda_stub(void* this_ptr, Args...args) {
            LambdaFunc* p = static_cast<LambdaFunc*>(this_ptr);
            return (p->operator())(args...);
        }

    };

    template<typename Result, typename...Args>
    class multicast_delegate_t<Result(Args...)> final : private base_delegate_t<Result(Args...)> {
        multicast_delegate_t(const multicast_delegate_t&) = delete;
        multicast_delegate_t& operator=(const multicast_delegate_t&) = delete;
    public:
        multicast_delegate_t() = default;
        ~multicast_delegate_t() = default;

        operator bool() const {
            return invocationList.size() < 1;
        }

    private:
        std::forward_list<std::unique_ptr<typename base_delegate_t<Result(Args...)>::func_stub_t>> invocationList;
    };
}

#endif //!VPSK_DELEGATE_HPP