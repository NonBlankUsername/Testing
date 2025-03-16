#pragma once

#include <lua.hpp>

class CExecution {
public:
    lua_State* NewThread(lua_State* L);
    void ExecuteLuaScript(lua_State* L, const char* script);
    void ExecutePythonScript(const char* script);
};

inline auto Execution = std::make_unique<CExecution>();