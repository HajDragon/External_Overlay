# External Overlay

A Windows overlay application built with C++ using Dear ImGui and DirectX 11.

## Features

- Transparent overlay window
- DirectX 11 rendering
- ImGui integration for UI elements
- Topmost window positioning
- Layered window attributes for transparency

## Technologies Used

- **C++20** - Core programming language
- **Dear ImGui** - Immediate mode GUI library
- **DirectX 11** - Graphics API for rendering
- **Win32 API** - Windows application framework

## Project Structure

```
External_Overlay/
??? src/
?   ??? main.cpp        # Main application entry point
?   ??? memory.h        # Memory utilities
?   ??? vector.h        # Vector utilities
??? external/
?   ??? imgui/          # Dear ImGui library files
??? build/              # Build outputs (ignored by git)
??? External_Overlay.* # Visual Studio project files
```

## Building

This project is configured for Visual Studio 2022 with C++20 support.

1. Open `External_Overlay.sln` in Visual Studio
2. Select your preferred configuration (Debug/Release)
3. Build the solution (Ctrl+Shift+B)

## Requirements

- Windows 10/11
- Visual Studio 2022 with C++20 support
- DirectX 11 compatible graphics hardware

## Usage

The application creates a transparent overlay window that can be used for displaying information over other applications. The overlay uses ImGui for rendering UI elements.

## License

This project is for educational/personal use.