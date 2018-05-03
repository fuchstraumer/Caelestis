#pragma once
#ifndef VPSK_TEMPLATE_UTILITY_FUNCTIONS_HPP
#define VPSK_TEMPLATE_UTILITY_FUNCTIONS_HPP
#include <tuple>
#include <type_traits>
namespace vpsk {

    template<typename...>
    struct typelist;

    // template<class> class Container is how we embed/use a templated container class in a template specifier
    // (oh my god templates why)
    template<typename Typelist, template<class> class Container>
    struct tuple_from_typelist;
    
    template<typename...Ts,template<class> class Container>
    struct tuple_from_typelist<typelist<Ts...>, Container> {
        using type = std::tuple<Container<Ts>...>;
    };

    template<typename Typelist, template<class> class Container>
    using tuple_from_typelist_t = typename tuple_from_typelist<Typelist, Container>::type;

    namespace detail {

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
    }
}

#endif //!VPSK_TEMPLATE_UTILITY_FUNCTIONS_HPP