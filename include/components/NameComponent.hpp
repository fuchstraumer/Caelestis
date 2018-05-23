#pragma once
#ifndef VPSK_NAME_COMPONENT_HPP
#define VPSK_NAME_COMPONENT_HPP
#include <string>
#include <string_view>
namespace vpsk {

    struct NameComponent {
        explicit NameComponent(std::string&& name);
        explicit NameComponent(const std::string& name);
        NameComponent(const NameComponent& other) noexcept;
        NameComponent(NameComponent&& other) noexcept;
        NameComponent& operator=(const NameComponent& other) noexcept;
        NameComponent& operator=(NameComponent&& other) noexcept;
        std::string_view Name;
    private:
        static std::string_view createView(std::string&& name);
        static std::string_view createView(const std::string& name);
    };
    
}

#endif // !VPSK_NAME_COMPONENT_HPP
