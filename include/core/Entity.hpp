#pragma once
#ifndef VPSK_ENTITY_HPP
#define VPSK_ENTITY_HPP
#include <cstdint>
#include <type_traits>
#include <atomic>

namespace vpsk {

    template<typename>
    struct EntityTraits;

    template<>
    struct EntityTraits<std::uint32_t> {
        using entity_type = std::uint32_t;
        using version_type = std::uint16_t;
        // Extract lower 24 bits representing the entity number / UUID
        static constexpr auto entity_mask = 0xFFFFF;
        // Extract upper 8 bits for the version
        static constexpr auto version_mask = 0xFFF;
        static constexpr auto entity_shift = 20;
    };

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

#endif //!VPSK_ENTITY_HPP