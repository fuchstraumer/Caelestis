#include "systems/LuaInstance.hpp"
#include "sol/state.hpp"
#include "sol/protected_function.hpp"
#include "sol/protected_function_result.hpp"
#include <unordered_map>
#include <string>
namespace vpsk {

    struct LuaInstanceData {
        std::unordered_map<std::string, sol::protected_function> registeredFunctions;
    };

    LuaInstance::LuaInstance() : impl(std::make_unique<sol::state>()) {
        impl->open_libraries(sol::lib::base, sol::lib::math, sol::lib::package, sol::lib::coroutine,
            sol::lib::string, sol::lib::table, sol::lib::bit32, sol::lib::ffi, sol::lib::jit);
    }

    LuaInstance::~LuaInstance() {}

    LuaInstance::LuaInstance(const LuaInstance& other) noexcept : LuaInstance() {
        // Don't explicitly copy the state: instead, create new state then copy environment from other.
        //sol::set_environment(sol::get_environment(*impl), *other.impl);
    }

    LuaInstance& LuaInstance::operator=(const LuaInstance& other) noexcept {
        //sol::set_environment(sol::get_environment(*impl), *other.impl);
        return *this;
    }

    lua_State* LuaInstance::State() noexcept {
        return impl->lua_state();
    }

}
