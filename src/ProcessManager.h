#pragma once

#include <windows.h>
#include <TlHelp32.h>
#include <string>
#include <optional>

// Modern C++20 approach for process management
class ProcessManager {
public:
    // Constructor and destructor
    ProcessManager() = default;
    ~ProcessManager();

    // Delete copy constructor and assignment operator (RAII)
    ProcessManager(const ProcessManager&) = delete;
    ProcessManager& operator=(const ProcessManager&) = delete;

    // Enable move constructor and assignment operator
    ProcessManager(ProcessManager&& other) noexcept;
    ProcessManager& operator=(ProcessManager&& other) noexcept;

    // Core functionality
    [[nodiscard]] bool AttachToProcess(const std::wstring& processName);
    [[nodiscard]] bool AttachToProcessById(DWORD processId);
    void DetachFromProcess();
    
    // Getters
    [[nodiscard]] HANDLE GetProcessHandle() const noexcept { return processHandle; }
    [[nodiscard]] DWORD GetProcessId() const noexcept { return processId; }
    [[nodiscard]] bool IsAttached() const noexcept { return processHandle != nullptr; }
    [[nodiscard]] std::wstring GetProcessName() const noexcept { return processName; }

    // Process validation
    [[nodiscard]] bool IsProcessRunning() const;
    [[nodiscard]] std::optional<DWORD> FindProcessByName(const std::wstring& name) const;

private:
    HANDLE processHandle = nullptr;
    DWORD processId = 0;
    std::wstring processName;

    // Helper methods
    void Reset();
    [[nodiscard]] bool OpenProcessWithAccess(DWORD pid);
};