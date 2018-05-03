#pragma once
#ifndef VPSK_ENTITY_ID_HPP
#define VPSK_ENTITY_ID_HPP
#include <cstdint>

namespace vpsk {

    class EntityManager;
    class Entity;

    template<typename T, typename Manager = EntityManager>
    class ComponentHandle;

    struct EntityID {

        constexpr EntityID() noexcept : handle(uint64_t(0)) {}
        constexpr explicit EntityID(uint64_t id) noexcept : handle(std::move(id)) {}
        constexpr explicit EntityID(uint32_t idx, uint32_t version) noexcept : handle(uint64_t(idx) | (uint64_t(version) << 32UL)) {}
        constexpr EntityID(const EntityID& other) noexcept : handle(other.handle) {}
        constexpr EntityID(EntityID&& other) noexcept : handle(std::move(other.handle)) {}

        constexpr EntityID& operator=(const EntityID& other) noexcept {
            handle = other.handle;
            return *this;
        }

        constexpr EntityID& operator=(EntityID&& other) noexcept {
            handle = std::move(other.handle);
            return *this;
        }

        constexpr const uint64_t& ID() const noexcept {
            return handle;
        }

        constexpr bool operator==(const EntityID& other) const noexcept {
            return handle == other.handle;
        }

        constexpr bool operator!=(const EntityID& other) const noexcept {
            return handle != other.handle;
        }

        constexpr bool operator!() const noexcept {
            return handle != uint64_t(0);
        }

        constexpr bool operator<(const EntityID& other) const noexcept {
            return handle < other.handle;
        }

        constexpr uint32_t Index() const noexcept {
            return handle & 0xffffffffUL;
        }

        constexpr uint32_t Version() const noexcept {
            return handle >> 32;
        }

    private:
        uint64_t handle;
        friend class EntityManager;
        friend class Entity;
    };

    constexpr static EntityID INVALID_ENTITY_ID;

}

#endif // !VPSK_ENTITY_ID_HPP
