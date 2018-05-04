#pragma once
#ifndef VPSK_REGISTRY_HPP
#define VPSK_REGISTRY_HPP

#include "Entity.hpp"
#include "Identifiers.hpp"
#include "storage/SparseSet.hpp"

namespace vpsk {

    template<typename EntityType>
    class Registry {
        using tag_family = Family<struct InternalRegistryTagFamily>;
        using component_family = Family<struct InternalRegistryComponentFamily>;
        using handler_family = Family<struct InternalRegistryHandlerFamily>;
    };

}

#endif //!VPSK_REGISTRY_HPP