#pragma once
#ifndef VPSK_DELEGATE_HPP
#define VPSK_DELEGATE_HPP

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

    template<typename Result, typename...Args>
    class delegate_t<Result(Args...)> : private base_delegate_t<Result(Args...)> {
    private:
        typename base_delegate_t<Result(Args...)>::invocation_element_t invocation;
    public:
        
        delegate_t() = default;

        bool operator==(const void* ptr) const {
            return (ptr == nullptr) && (IsEmpty());
        }

        bool operator!=(const void* ptr) const {
            return (ptr != nullptr) || (!IsEmpty());
        }

        delegate_t(const delegate_t& other) noexcept {
            other.invocation.copy_to(invocation);
        }

        delegate_t& operator=(const delegate_t& other) noexcept {
            other.invocation.copy_to(invocation);
            return *this;
        }

        delegate_t(delegate_t&& other) noexcept : invocation(std::move(other.invocation)) {}

        delegate_t& operator=(delegate_t&& other) noexcept {
            invocation = std::move(other.invocation);
        }

        template<typename LambdaFunc>
        delegate_t(const LambdaFunc& func) {
            assign(static_cast<void*>(&func), )
        }


        // i.e like if (!ptr_object), returns true when object doesn't "exist" and false
        // when it exists. thus if (!my_delegate_t) == if (my_delegate_t.invocation.stub == nullptr)
        bool operator!() const noexcept {
            return invocation.stub == nullptr;
        }

        bool IsEmpty() const noexcept {
            return invocation.stub == nullptr;
        }

    private:

        void assign(void* object_ptr, typename base_delegate_t<Result(Args...)>::func_stub_t _stub) {
            this->invocation.object = object_ptr;
            this->invocation.stub = _stub;
        }

        template<typename LambdaFunc>
        static Result lambdaStub(void* this_ptr, Args&&...args) {
            LambdaFunc* p = static_cast<LambdaFunc*>(this_ptr);
            return (p->operator())(std::forward<Args>(args)...);
        }
    };


}

#endif //!VPSK_DELEGATE_HPP