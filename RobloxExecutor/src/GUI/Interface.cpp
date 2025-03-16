#include "Interface.hpp"
#include <imgui.h>
#include <Windows.h>
#include "Execution/Execution.hpp"

void Interface::Render() {
    ImGui::Begin("Roblox Executor");

    ImGui::Text("Script Executor");
    static char script[1024] = "";
    ImGui::InputTextMultiline("##script", script, sizeof(script));

    if (ImGui::Button("Execute Lua")) {
        Execution->ExecuteLuaScript(Manager->GetLuaState(), script);
    }

    if (ImGui::Button("Execute Python")) {
        Execution->ExecutePythonScript(script);
    }

    ImGui::End();
}