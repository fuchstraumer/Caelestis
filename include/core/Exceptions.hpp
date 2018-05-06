#pragma once
#ifndef VPSK_ENTITY_SYSTEM_EXCEPTIONS_HPP
#define VPSK_ENTITY_SYSTEM_EXCEPTIONS_HPP
#include "CommonDef.hpp"
#include <stdexcept>

namespace vpsk {

    enum class entity_system_error_t {
        BadEntity,
        InvalidComponent,
        InvalidTag
    };

    struct bad_entity : std::logic_error {
        using std::logic_error::logic_error;
    };

    struct invalid_component : std::logic_error {
        using std::logic_error::logic_error;
    };

    struct invalid_tag : std::logic_error {
        using std::logic_error::logic_error;
    };

}

#endif // !VPSK_ENTITY_SYSTEM_EXCEPTIONS_HPP
