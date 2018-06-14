#pragma once
#ifndef VPSK_LUA_INSTANCE_HPP
#define VPSK_LUA_INSTANCE_HPP
#include <memory>

struct lua_State;

namespace sol {
    class state;
    class state_view;
    struct protected_function_result;
}

namespace vpsk {

    struct LuaInstanceData;

    class LuaInstance {
    public:

        LuaInstance();
        ~LuaInstance();
        LuaInstance(const LuaInstance&);
        LuaInstance& operator=(const LuaInstance&);
        lua_State* State() noexcept;
        
    private:
        std::unique_ptr<LuaInstanceData> instance;
        std::unique_ptr<sol::state> impl;
    };

}

#endif //!VPSK_LUA_INSTANCE_HPP
