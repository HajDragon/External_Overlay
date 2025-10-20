#include "MemoryManager.h"
#include <iostream>

MemoryManager::MemoryManager(ProcessManager& procManager) 
    : processManager(procManager) {
}

std::optional<std::uint32_t> MemoryManager::ReadUInt32(uintptr_t address) const {
    return ReadMemory<std::uint32_t>(address);
}

bool MemoryManager::WriteUInt32(uintptr_t address, std::uint32_t value) const {
    return WriteMemory<std::uint32_t>(address, value);
}

std::optional<std::uint64_t> MemoryManager::ReadUInt64(uintptr_t address) const {
    return ReadMemory<std::uint64_t>(address);
}

bool MemoryManager::WriteUInt64(uintptr_t address, std::uint64_t value) const {
    return WriteMemory<std::uint64_t>(address, value);
}

std::optional<float> MemoryManager::ReadFloat(uintptr_t address) const {
    return ReadMemory<float>(address);
}

bool MemoryManager::WriteFloat(uintptr_t address, float value) const {
    return WriteMemory<float>(address, value);
}

bool MemoryManager::IsValidAddress(uintptr_t address) const {
    if (!processManager.IsAttached()) {
        return false;
    }
    
    // Try to read a single byte to validate the address
    MEMORY_BASIC_INFORMATION memInfo;
    SIZE_T result = VirtualQueryEx(processManager.GetProcessHandle(), 
                                   reinterpret_cast<LPCVOID>(address), 
                                   &memInfo, 
                                   sizeof(memInfo));
    
    if (result == 0) {
        return false;
    }
    
    // Check if the memory is accessible for reading/writing
    return (memInfo.State == MEM_COMMIT) && 
           (memInfo.Protect & (PAGE_READWRITE | PAGE_READONLY | PAGE_EXECUTE_READWRITE));
}

bool MemoryManager::ReadRawMemory(uintptr_t address, void* buffer, size_t size) const {
    if (!processManager.IsAttached()) {
        return false;
    }
    
    if (!buffer || size == 0) {
        return false;
    }
    
    SIZE_T bytesRead = 0;
    BOOL success = ReadProcessMemory(processManager.GetProcessHandle(),
                                     reinterpret_cast<LPCVOID>(address),
                                     buffer,
                                     size,
                                     &bytesRead);
    
    return success && (bytesRead == size);
}

bool MemoryManager::WriteRawMemory(uintptr_t address, const void* buffer, size_t size) const {
    if (!processManager.IsAttached()) {
        return false;
    }
    
    if (!buffer || size == 0) {
        return false;
    }
    
    SIZE_T bytesWritten = 0;
    BOOL success = WriteProcessMemory(processManager.GetProcessHandle(),
                                      reinterpret_cast<LPVOID>(address),
                                      buffer,
                                      size,
                                      &bytesWritten);
    
    return success && (bytesWritten == size);
}