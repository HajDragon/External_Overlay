#include "ProcessManager.h"
#include <iostream>

ProcessManager::~ProcessManager() {
    DetachFromProcess();
}

ProcessManager::ProcessManager(ProcessManager&& other) noexcept
    : processHandle(other.processHandle), processId(other.processId), processName(std::move(other.processName)) {
    other.Reset();
}

ProcessManager& ProcessManager::operator=(ProcessManager&& other) noexcept {
    if (this != &other) {
        DetachFromProcess();
        processHandle = other.processHandle;
        processId = other.processId;
        processName = std::move(other.processName);
        other.Reset();
    }
    return *this;
}

bool ProcessManager::AttachToProcess(const std::wstring& name) {
    auto pid = FindProcessByName(name);
    if (!pid.has_value()) {
        return false;
    }
    
    return AttachToProcessById(pid.value());
}

bool ProcessManager::AttachToProcessById(DWORD pid) {
    DetachFromProcess(); // Clean up any existing attachment
    
    if (!OpenProcessWithAccess(pid)) {
        return false;
    }
    
    processId = pid;
    processName = L"Process_" + std::to_wstring(pid); // Default name, could be enhanced
    return true;
}

void ProcessManager::DetachFromProcess() {
    if (processHandle) {
        CloseHandle(processHandle);
    }
    Reset();
}

bool ProcessManager::IsProcessRunning() const {
    if (!processHandle) {
        return false;
    }
    
    DWORD exitCode;
    if (!GetExitCodeProcess(processHandle, &exitCode)) {
        return false;
    }
    
    return exitCode == STILL_ACTIVE;
}

std::optional<DWORD> ProcessManager::FindProcessByName(const std::wstring& name) const {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return std::nullopt;
    }
    
    PROCESSENTRY32W processEntry = {};
    processEntry.dwSize = sizeof(PROCESSENTRY32W);
    
    if (!Process32FirstW(snapshot, &processEntry)) {
        CloseHandle(snapshot);
        return std::nullopt;
    }
    
    do {
        if (name == processEntry.szExeFile) {
            DWORD pid = processEntry.th32ProcessID;
            CloseHandle(snapshot);
            return pid;
        }
    } while (Process32NextW(snapshot, &processEntry));
    
    CloseHandle(snapshot);
    return std::nullopt;
}

void ProcessManager::Reset() {
    processHandle = nullptr;
    processId = 0;
    processName.clear();
}

bool ProcessManager::OpenProcessWithAccess(DWORD pid) {
    constexpr DWORD desiredAccess = PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION;
    
    processHandle = OpenProcess(desiredAccess, FALSE, pid);
    return processHandle != nullptr;
}