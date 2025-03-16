#include <Windows.h>
#include <pybind11/embed.h>
#include "RBX.hpp"
#include "ThreadPool.hpp"
#include "Environment/Environment.hpp"
#include "Execution/Execution.hpp"
#include "GUI/Interface.hpp"

namespace py = pybind11;

extern "C" __declspec(dllexport) LRESULT InitHook(int Code, WPARAM WParam, LPARAM LParam) { return CallNextHookEx(nullptr, Code, WParam, LParam); }

void MainThread(HMODULE Module) {
    RBX::Scheduler->Initialize();
    py::scoped_interpreter guard{}; // Start the Python interpreter

    auto ScriptContext = RBX::Scheduler->GetScriptContext();
    auto GlobalState = ScriptContext + Update::ScriptContext::GlobalState;

    uintptr_t StateIndex[] = { 0 };
    uintptr_t ActorIndex[] = { 0, 0 };

    lua_State* L = RBX::DecryptState(RBX::GetGlobalStateForInstance(GlobalState, StateIndex, ActorIndex) + Update::ScriptContext::DecryptState);

    lua_State* ExploitThread = Execution->NewThread(L);
    luaL_sandboxthread(ExploitThread);

    Manager->SetLuaState(ExploitThread);
    RBX::Scheduler->HookJob("Heartbeat");

    Environment->Initialize(Manager->GetLuaState());

    Interface gui;
    while (true) {
        gui.Render();
        Sleep(10);
    }
}

bool __stdcall DllMain(HMODULE Module, uintptr_t Reason, void*) {
    if (Reason == DLL_PROCESS_ATTACH)
        ThreadPool->Run(MainThread, Module);

    return true;
}