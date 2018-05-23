#pragma once
#ifndef VPSK_HASHED_STRING_HPP
#define VPSK_HASHED_STRING_HPP
#include <string_view>

namespace vpsk {

    class HashedString {

        static constexpr std::uint64_t offset = 14695981039346656037ull;
        static constexpr std::uint64_t prime = 1099511628211ull;

        static constexpr std::uint64_t hash_helper(std::uint64_t val, std::string_view str) noexcept {
            return str[0] == 0 ? val : hash_helper((val^str[0])*prime, std::string_view(str.data() + 1, str.length()));
        }

    public:
        using hash_type = std::uint64_t;

        template<std::size_t N>
        constexpr HashedString(const char (&str)[N]) noexcept : value(hash_helper(offset, std::string_view(str, N))), view(str, N) {}

        explicit constexpr HashedString(std::string_view _view) noexcept : value(hash_helper(offset, _view)), view(_view) {}

        constexpr operator const char*() const noexcept {
            return view.data();
        }

        constexpr operator hash_type() const noexcept {
            return value;
        }

        constexpr bool operator==(const HashedString& other) const noexcept {
            return value == other.value;
        }

        constexpr bool operator!=(const HashedString& other) const noexcept {
            return value != other.value;
        }

    private:
        std::uint64_t value;
        std::string_view view;
    };

}

#endif //!VPSK_HASHED_STRING_HPP
