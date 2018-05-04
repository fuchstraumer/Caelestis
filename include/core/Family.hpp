#pragma once
#ifndef VPSK_FAMILY_ID_GENERATOR_HPP
#define VPSK_FAMILY_ID_GENERATOR_HPP
#include <type_traits>
#include <atomic>

namespace vpsk {

    template<typename...>
    class Family {
        static std::atomic<std::size_t> id;

        template<typename...>
        static std::size_t getID() noexcept {
            static const std::size_t value = id.fetch_add(1);
            return value;
        }

    public:
        using family_type = std::size_t;

        // Used to return a unique identifier for the given type
        template<typename...Ts>
        inline static family_type Type() noexcept {
            return getID<std::decay_t<Ts>...>();
        }

    };

    template<typename...Ts>
    std::atomic<std::size_t> Family<Ts...>::id{};

}

#endif //!VPSK_FAMILY_ID_GENERATOR_HPP