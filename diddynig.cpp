#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <psapi.h>
#include <cstdint>
#include <regex>
#include "Offsets.hpp"

uintptr_t readMemory(HANDLE process, uintptr_t address, size_t size) {
    uintptr_t value = 0;
    if (!ReadProcessMemory(process, (LPCVOID)address, &value, size, NULL)) {
        std::cerr << "Error reading memory at 0x" << std::hex << address << ": " << GetLastError() << std::dec << std::endl;
        return 0;
    }
    return value;
}

bool readString(HANDLE process, uintptr_t address, char* buffer, size_t size) {
    return ReadProcessMemory(process, (LPCVOID)address, buffer, size, NULL);
}

DWORD getProcessIdByName(const std::string& processName) {
    DWORD pid = 0;
    DWORD processIds[1024], cbNeeded, processCount;

    if (EnumProcesses(processIds, sizeof(processIds), &cbNeeded)) {
        processCount = cbNeeded / sizeof(DWORD);
        for (unsigned int i = 0; i < processCount; ++i) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processIds[i]);
            if (hProcess) {
                char szProcessName[MAX_PATH] = { 0 };
                if (GetModuleBaseNameA(hProcess, NULL, szProcessName, sizeof(szProcessName))) {
                    if (processName == szProcessName) {
                        pid = processIds[i];
                        CloseHandle(hProcess);
                        return pid;
                    }
                }
                CloseHandle(hProcess);
            }
        }
    }
    return 0;
}

uintptr_t getModuleBaseAddress(DWORD pid, const char* moduleName) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) return 0;

    HMODULE hMods[1024];
    DWORD cbNeeded;
    if (EnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_64BIT)) {
        for (size_t i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            char szModName[MAX_PATH];
            if (GetModuleBaseNameA(hProcess, hMods[i], szModName, sizeof(szModName))) {
                if (_stricmp(szModName, moduleName) == 0) {
                    uintptr_t baseAddress = (uintptr_t)hMods[i];
                    CloseHandle(hProcess);
                    return baseAddress;
                }
            }
        }
    }
    CloseHandle(hProcess);
    return 0;
}

std::vector<uintptr_t> GetChildren(HANDLE process, uintptr_t InstanceAddress) {
    std::vector<uintptr_t> children;
    uintptr_t childrenStart = readMemory(process, InstanceAddress + Offsets::Children, 8);
    if (childrenStart == 0) return {};
    uintptr_t childrenEnd = readMemory(process, childrenStart + 8, 8);
    uintptr_t offsetPerChild = 0x10;
    uintptr_t currentChildAddress = readMemory(process, childrenStart, 8);

    for (int i = 0; i < 9000; ++i) {
        if (i == 8999) {
            std::cout << "WARNING: Too many children, possible invalid data." << std::endl;
        }
        if (currentChildAddress == childrenEnd) {
            break;
        }
        children.push_back(readMemory(process, currentChildAddress, 8));
        currentChildAddress += offsetPerChild;
    }
    return children;
}

std::vector<uintptr_t> GetDescendants(HANDLE process, uintptr_t instanceAddress) {
    std::vector<uintptr_t> descendants;
    auto children = GetChildren(process, instanceAddress);

    for (auto child : children) {
        descendants.push_back(child);
        auto childDescendants = GetDescendants(process, child);
        descendants.insert(descendants.end(), childDescendants.begin(), childDescendants.end());
    }
    return descendants;
}

bool isValidText(const std::string& text) {
    return std::regex_match(text, std::regex("^[a-zA-Z0-9_ ]+$"));
}

uintptr_t findGuiObject(HANDLE process, uintptr_t parentAddress, const std::string& targetName) {
    auto children = GetChildren(process, parentAddress);

    for (const auto& child : children) {
        char childName[256] = { 0 };
        uintptr_t childNameAddress = readMemory(process, child + Offsets::Name, 8);
        if (childNameAddress && readString(process, childNameAddress, childName, sizeof(childName))) {
            std::string name(childName);
            if (isValidText(name) && name == targetName) {
                return child;
            }
        }
    }
    return 0;
}

int main() {
    std::string processName = "RobloxPlayerBeta.exe";
    DWORD pid = getProcessIdByName(processName);
    if (pid == 0) {
        std::cerr << "Roblox process not found!" << std::endl;
        return 1;
    }

    HANDLE process = OpenProcess(PROCESS_VM_READ, FALSE, pid);
    if (!process) {
        std::cerr << "Unable to open process!" << std::endl;
        return 1;
    }

    uintptr_t baseAddress = getModuleBaseAddress(pid, "RobloxPlayerBeta.exe");
    if (!baseAddress) {
        std::cerr << "Failed to get base address!" << std::endl;
        CloseHandle(process);
        return 1;
    }

    std::cout << "Base Address: 0x" << std::hex << baseAddress << std::endl;

    uintptr_t fakeDataModel = readMemory(process, baseAddress + Offsets::FakeDataModel, 8);
    if (!fakeDataModel) {
        std::cerr << "Invalid fakeDataModel pointer!" << std::endl;
        CloseHandle(process);
        return 1;
    }

    std::cout << "fakeDataModel Address: 0x" << std::hex << fakeDataModel << std::endl;

    uintptr_t dataModel = readMemory(process, fakeDataModel + Offsets::DataModel, 8);
    if (!dataModel) {
        std::cerr << "Invalid dataModel pointer!" << std::endl;
        CloseHandle(process);
        return 1;
    }

    std::cout << "DataModel Address: 0x" << std::hex << dataModel << std::endl;

    uintptr_t workspace = readMemory(process, dataModel + Offsets::Workspace, 8);
    if (!workspace) {
        std::cerr << "Invalid workspace pointer!" << std::endl;
        CloseHandle(process);
        return 1;
    }

    std::cout << "Workspace Address: 0x" << std::hex << workspace << std::endl;

    uintptr_t coreGui = findGuiObject(process, dataModel, "CoreGui");
    if (!coreGui) {
        std::cerr << "CoreGui not found!" << std::endl;
        CloseHandle(process);
        return 1;
    }
    std::cout << "CoreGui Address: 0x" << std::hex << coreGui << std::endl;

    uintptr_t robloxGui = findGuiObject(process, coreGui, "RobloxGui");
    if (!robloxGui) {
        std::cerr << "RobloxGui not found!" << std::endl;
        CloseHandle(process);
        return 1;
    }
    std::cout << "RobloxGui Address: 0x" << std::hex << robloxGui << std::endl;

    uintptr_t modules = findGuiObject(process, robloxGui, "Modules");
    if (!modules) {
        std::cerr << "Modules not found!" << std::endl;
        CloseHandle(process);
        return 1;
    }
    std::cout << "Modules Address: 0x" << std::hex << modules << std::endl;

    uintptr_t common = findGuiObject(process, modules, "Common");
    if (!common) {
        std::cerr << "Common not found!" << std::endl;
        CloseHandle(process);
        return 1;
    }
    std::cout << "Common Address: 0x" << std::hex << common << std::endl;

    uintptr_t httpRequest = findGuiObject(process, common, "httpRequest");
    if (!httpRequest) {
        std::cerr << "httpRequest not found!" << std::endl;
        CloseHandle(process);
        return 1;
    }
    std::cout << "httpRequest Address: 0x" << std::hex << httpRequest << std::endl;

    CloseHandle(process);
    return 0;
}
