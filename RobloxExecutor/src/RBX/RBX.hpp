#pragma once

#include <lua.hpp>

namespace RBX {
    class SchedulerClass {
    public:
        void Initialize();
        uintptr_t GetScriptContext();
        uintptr_t HookJob(const char* jobName);
    };

    inline auto Scheduler = std::make_unique<SchedulerClass>();

    uintptr_t GetGlobalStateForInstance(uintptr_t GlobalState, uintptr_t* StateIndex, uintptr_t* ActorIndex);
    lua_State* DecryptState(uintptr_t DecryptState);
}