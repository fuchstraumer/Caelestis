#pragma once
#ifndef VPSK_ENTITY_HPP
#define VPSK_ENTITY_HPP
#include "EntityID.hpp"
#include "Event.hpp"
#include <memory>
#include <type_traits>
#include <bitset>
#include <tuple>

namespace vpsk {

    constexpr size_t MAX_COMPONENTS = 128;

    class Entity {
    public:

        constexpr Entity() noexcept : id(EntityID()), parent(nullptr) {}
        constexpr Entity(const Entity& other) noexcept : id(other.id), parent(other.parent) {}
        constexpr Entity(Entity&& other) noexcept : id(std::move(other.id)), parent(std::move(other.parent)) {}
        Entity(EntityManager* manager, EntityID _id) : parent(manager), id(std::move(_id)) {}
        ~Entity() noexcept {}

        constexpr Entity& operator=(const Entity& other) noexcept {
            parent = other.parent;
            id = other.id;
            return *this;
        }

        constexpr Entity& operator=(Entity&& other) noexcept {
            parent = std::move(other.parent);
            id = std::move(other.id);
            return *this;
        }

        bool Valid() const;
        void Invalidate();
        void Destroy();

        EntityID ID() const {
            return id;
        }

        operator bool() const noexcept {
            return Valid();
        }

        constexpr bool operator==(const Entity& other) const noexcept {
            return parent == other.parent && id == other.id;
        }

        constexpr bool operator!=(const Entity& other) const noexcept {
            return !(other == *this);
        }

        constexpr bool operator<(const Entity& other) const noexcept {
            return id < other.id;
        }

        template<typename T, typename...Args>
        constexpr ComponentHandle<T> Assign(Args&&...args);
        template<typename T>
        constexpr ComponentHandle<T> AssignFromCopy(const T& component);
        template<typename T>
        constexpr ComponentHandle<T> Replace(Args&&...args);
        template<typename T>
        constexpr void Remove();
        template<typename T, typename = std::enable_if_t<!std::is_const<T>::value>>
        constexpr ComponentHandle<T> Component();
        template<typename T, typename = std::enable_if_t<std::is_const<T>::value>>
        constexpr ComponentHandle<T, const EntityManager> Component() const;
        template<typename...Ts>
        constexpr std::tuple<ComponentHandle<Ts>...> Components();
        template<typename...Ts>
        constexpr std::tuple<ComponentHandle<const Ts, const EntityManager>...> Components() const;
        template<typename T>
        constexpr bool HasComponent() const;
        template<typename T, typename...Args>
        void Unpack(ComponentHandle<T>& item_a, ComponentHandle<Args>&...args);

        std::bitset<MAX_COMPONENTS> ComponentMask() const;

    private:
        EntityManager * parent = nullptr;
        EntityID id = INVALID_ENTITY_ID;
    };

    template<typename T, typename ManagerType>
    class ComponentHandle {
    public:

        typedef T ComponentType;

        constexpr ComponentHandle() noexcept : manager(nullptr) {}
        constexpr ComponentHandle(const ComponentHandle& other) noexcept : manager(other.manager), id(other.id) {}
        constexpr ComponentHandle(ComponentHandle&& other) noexcept : manager(std::move(other.manager)), id(std::move(other.id)) {}
        constexpr ComponentHandle& operator=(const ComponentHandle& other) noexcept {
            manager = other.manager;
            id = other.id;
            return *this;
        }

        constexpr ComponentHandle& operator=(ComponentHandle&& other) noexcept {
            manager = std::move(other.manager);
            id = std::move(other.id);
            return *this;
        }

        T* operator->();
        const T* operator->() const;
        T& operator*();
        const T& operator*() const;
        T* get();
        const T* get() const;

        Entity GetEntity() const;

        constexpr bool operator==(const ComponentHandle& other) const noexcept {
            return manager == other.manager && id == other.id;
        }

        constexpr bool operator!=(const ComponentHandle& other) const noexcept {
            return !(*this == other);
        }
        
        bool Valid() const;
        operator bool() const;

    private:
        friend class EntityManager;

        constexpr ComponentHandle(ManagerType* _manager, EntityID _id) noexcept : manager(_manager), id(std::move(_id)) {}

        ManagerType* manager;
        EntityID id;
    };

    struct BaseComponent {
        using Family = size_t;
        static Family& FamilyCounter() {
            static Family family = 0;
            return family;
        }
    };



}

#endif //!VPSK_ENTITY_HPP