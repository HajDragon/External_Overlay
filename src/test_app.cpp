// Test program to simulate the "elden ring" reference application
// This creates a simple console application with a rune variable at a known address
// Compile this separately as a test target

#include <iostream>
#include <thread>
#include <chrono>
#include <windows.h>

int main() {
    // Create a variable that will act as our "rune" value
    static std::uint32_t runes = 1000; // Starting with 1000 runes
    
    std::cout << "Elden Ring Reference Application Started\n";
    std::cout << "Process Name: eldenring.exe (PID: " << GetCurrentProcessId() << ")\n";
    std::cout << "Rune Address: 0x" << std::hex << reinterpret_cast<uintptr_t>(&runes) << std::dec << "\n";
    std::cout << "Initial Runes: " << runes << "\n\n";
    
    std::cout << "Instructions:\n";
    std::cout << "1. Note the process name and rune address above\n";
    std::cout << "2. Use your overlay to connect to this process\n";
    std::cout << "3. Try different addresses in the dropdown to find the working one\n";
    std::cout << "4. Watch the rune value change in real-time below\n\n";
    
    std::uint32_t lastRunes = runes;
    
    while (true) {
        if (runes != lastRunes) {
            std::cout << "Runes changed! New value: " << runes << "\n";
            lastRunes = runes;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return 0;
}