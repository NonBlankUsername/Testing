#include "Execution.hpp"
#include <pybind11/embed.h>

namespace py = pybind11;

lua_State* CExecution::NewThread(lua_State* L) {
    lua_State* newThread = lua_newthread(L);
    return newThread;
}

void CExecution::ExecuteLuaScript(lua_State* L, const char* script) {
    if (luaL_dostring(L, script) != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        printf("Lua Error: %s\n", error);
        lua_pop(L, 1);
    }
}

void CExecution::ExecutePythonScript(const char* script) {
    try {
        py::exec(script);
    }
    catch (const std::exception& e) {
        printf("Python Error: %s\n", e.what());
    }
}