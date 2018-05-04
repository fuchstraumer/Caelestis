#pragma once
#ifndef VPSK_ENTITY_HPP
#define VPSK_ENTITY_HPP
#include <cstdint>

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

}

#endif //!VPSK_ENTITY_HPP