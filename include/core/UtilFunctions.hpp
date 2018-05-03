#pragma once
#ifndef VPSK_TEMPLATE_UTILITY_FUNCTIONS_HPP
#define VPSK_TEMPLATE_UTILITY_FUNCTIONS_HPP
#include <tuple>
#include <type_traits>
namespace vpsk {

    namespace detail {

        template<typename...>
        struct typelist;

        // template<class> class Container is how we embed/use a templated container class in a template specifier
        // (oh my god templates why)
        template<typename Typelist, template<class> class Container>
        struct tuple_from_typelist;

        template<typename...Ts, template<class> class Container>
        struct tuple_from_typelist<typelist<Ts...>, Container> {
            using type = std::tuple<Container<Ts>...>;
        };

        template<typename Typelist, template<class> class Container>
        using tuple_from_typelist_t = typename tuple_from_typelist<Typelist, Container>::type;

        template<typename T>
        using is_not_t = std::integral_constant<bool, !T::value>;
        template<typename T, typename U>
        using and_t = std::integral_constant<bool, T::value && U::value>;
        template<typename T, typename U>
        using or_t = std::integral_constant<bool, T::value || U::value>;
        template<bool...>
        struct bool_pack;

        template<typename...Ts>
        struct and_all_t : std::is_same<bool_pack<true, Ts::value...>, bool_pack<Ts::value..., true>> {};
        template<typename...Ts>
        struct or_all_t : is_not_t<std::is_same<bool_pack<false, Ts::value...>, bool_pack<Ts::value..., false>>> {};

        template<typename T, typename Typelist>
        struct typelist_has_type;
        template<typename T, typename...Ts>
        struct typelist_has_type<T, typelist<Ts...>> : or_all_t<std::is_same<T, Ts>...> {};
        template<typename T, typename U>
        constexpr bool typelist_has_type_v = typelist_has_type<T, U>::value;

        template<typename Typelist>
        struct is_typelist_unique;
        template<>
        struct is_typelist_unique<typelist<>> : std::true_type {};
        template<typename T, typename...Ts>
        struct is_typelist_unique<typelist<T,Ts...>> : and_t<is_not_t<typelist_has_type<T,typelist<Ts...>>>, is_typelist_unique<typelist<Ts...>>> {};
        template<typename T>
        constexpr bool is_typelist_unique_v = is_typelist_unique<T>::value;

        template<typename T, typename Typelist>
        struct typelist_index;
        template<typename T, typename...Ts>
        struct typelist_index<T,typelist<T,Ts...>> : std::integral_constant<std::size_t, 0> {};
        template<typename T, typename U, typename...Ts>
        struct typelist_index<T,typelist<U,Ts...>> : std::integral_constant<std::size_t, 1 + typelist_index<T,typelist<Ts...>>::value> {};
        template<typename T, typename U>
        constexpr std::size_t typelist_index_v = typelist_index<T, U>::value;

        template<std::size_t idx, typename Typelist>
        struct typelist_type;
        template<typename T, typename...Ts>
        struct typelist_type<0, typelist<T, Ts...>> {
            using type = T;
        };
        template<std::size_t idx, typename T, typename...Ts>
        struct typelist_type<Idx, typelist<T, Ts...>> {
            using type = typename typelist_type<Idx - 1, typelist<Ts...>>::type;
        };
        template<std::size_t I, typename T>
        using typelist_type_t = typename typelist_type<I, T>::type;

        template<typename T, typename U>
        struct typelist_concat;
        template<typename...Ts,typename...Us>
        struct typelist_concat<typelist<Ts...>, typelist<Us...>> {
            using type = typelist<Ts..., Us...>;
        };
        template<typename T, typename U>
        using typelist_concat_T = typename typelist_concat<T, U>::type;

        template<typename T, typename U>
        struct typelist_intersection;
        template<typename...Us>
        struct typelist_intersection<typelist<>, typelist<Us...>> {
            using type = typelist<>;
        };
        template <typename T, typename... Ts, typename... Us>
        struct typelist_intersection<typelist<T, Ts...>, typelist<Us...>> {
            using type = std::conditional_t<typelist_has_type_v<T, typelist<Us...>>,
                typelist_concat_t<typelist<T>, typename typelist_intersection<typelist<Ts...>, typelist<Us...>>::type>,
                typename typelist_intersection<typelist<Ts...>, typelist<Us...>>::type>;
        };
        template<typename T, typename U>
        using typelist_intersection_t = typename typelist_intersection<T, U>::type;
    }
}

#endif //!VPSK_TEMPLATE_UTILITY_FUNCTIONS_HPP