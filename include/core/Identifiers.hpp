#pragma once
#ifndef VPSK_IDENTIFIERS_HPP
#define VPSK_IDENTIFIERS_HPP
#include <cstdint>
#include <utility>
#include <type_traits>

namespace vpsk {

    namespace detail {

        template<typename...Ts>
        struct identifier final : identifier<Ts>... {
            using identifier_type = std::size_t;

            template<std::size_t...Indices>
            constexpr identifier(std::index_sequence<Indices...>) : identifier<Ts>{std::index_sequence<Indices>{}}... {}

            template<typename Type>
            constexpr std::size_t get() const {
                return identifier<std::decay_t<Type>>::get();
            }

        };

        template<typename Type>
        struct identifier<Type> {
            using identifier_type = std::size_t;

            template<std::size_t Index>
            constexpr identifier(std::index_sequence<Index>) : index{Index} {}

            constexpr std::size_t get() const {
                return index;
            }

        private:
            const std::size_t index;
        };

    }

    template<typename...Ts>
    constexpr auto Identifier = detail::identifier<std::decay_t<Types>...>{std::make_index_sequence<sizeof...(Types)>{}};
    
}

#endif //!VPSK_IDENTIFIERS_HPP