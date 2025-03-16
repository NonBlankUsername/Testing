#include "RBX.hpp"

void RBX::SchedulerClass::Initialize() {
    // Initialize Scheduler
}

uintptr_t RBX::SchedulerClass::GetScriptContext() {
    // Return ScriptContext
    return 0;
}

uintptr_t RBX::SchedulerClass::HookJob(const char* jobName) {
    // Hook Job
    return 0;
}

uintptr_t RBX::GetGlobalStateForInstance(uintptr_t GlobalState, uintptr_t* StateIndex, uintptr_t* ActorIndex) {
    // Get Global State for Instance
    return 0;
}

lua_State* RBX::DecryptState(uintptr_t DecryptState) {
    // Decrypt State
    return nullptr;
}