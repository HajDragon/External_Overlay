#pragma once

#include "ProcessManager.h"
#include <cstdint>
#include <type_traits>
#include <optional>

// Modern C++20 memory manipulation class
class MemoryManager {
public:
    // Constructor takes a reference to ProcessManager
    explicit MemoryManager(ProcessManager& procManager);
    
    // Default destructor is sufficient
    ~MemoryManager() = default;
    
    // Delete copy constructor and assignment operator
    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;
    
    // Enable move constructor and assignment operator
    MemoryManager(MemoryManager&& other) noexcept = default;
    MemoryManager& operator=(MemoryManager&& other) noexcept = default;

    // Template-based memory operations for type safety
    template<typename T>
    [[nodiscard]] std::optional<T> ReadMemory(uintptr_t address) const;
    
    template<typename T>
    [[nodiscard]] bool WriteMemory(uintptr_t address, const T& value) const;
    
    // Specialized methods for common types
    [[nodiscard]] std::optional<std::uint32_t> ReadUInt32(uintptr_t address) const;
    [[nodiscard]] bool WriteUInt32(uintptr_t address, std::uint32_t value) const;
    
    [[nodiscard]] std::optional<std::uint64_t> ReadUInt64(uintptr_t address) const;
    [[nodiscard]] bool WriteUInt64(uintptr_t address, std::uint64_t value) const;
    
    [[nodiscard]] std::optional<float> ReadFloat(uintptr_t address) const;
    [[nodiscard]] bool WriteFloat(uintptr_t address, float value) const;
    
    // Address validation
    [[nodiscard]] bool IsValidAddress(uintptr_t address) const;
    
    // Get process manager reference
    [[nodiscard]] const ProcessManager& GetProcessManager() const noexcept { return processManager; }

private:
    ProcessManager& processManager;
    
    // Helper methods
    [[nodiscard]] bool ReadRawMemory(uintptr_t address, void* buffer, size_t size) const;
    [[nodiscard]] bool WriteRawMemory(uintptr_t address, const void* buffer, size_t size) const;
};

// Template implementations
template<typename T>
std::optional<T> MemoryManager::ReadMemory(uintptr_t address) const {
    static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
    
    T value{};
    if (ReadRawMemory(address, &value, sizeof(T))) {
        return value;
    }
    return std::nullopt;
}

template<typename T>
bool MemoryManager::WriteMemory(uintptr_t address, const T& value) const {
    static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
    
    return WriteRawMemory(address, &value, sizeof(T));
}