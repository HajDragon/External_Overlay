# External Overlay

A Windows overlay application built with C++ using Dear ImGui and DirectX 11.
This application allows user to manipulate the amount of runes in Elden Ring and display custom images as an overlay.

## Features

- **Transparent overlay window** - Renders over other applications
- **DirectX 11 rendering** - Hardware-accelerated graphics
- **ImGui integration** - Modern immediate-mode GUI
- **Topmost window positioning** - Always appears above other windows
- **Layered window attributes** - Full transparency support
- **Dynamic image management** - Load and display any number of custom images
- **User-settable image paths** - Browse and select images through file dialog
- **Overlay visibility toggle** - Press **F2** to show/hide the overlay
- **Click-through mode** - Press **F1** to toggle between interactive and click-through modes
- **Memory manipulation** - Read and write process memory for game hacking
- **Dynamic memory address support** - Find and use changing memory addresses

## Controls

- **F2** - Toggle overlay visibility (show/hide)
- **F1** - Toggle between interactive mode and click-through mode
  - Interactive: You can click and interact with the overlay
  - Click-through: Mouse input passes through to applications below

## New Dynamic Image System

The overlay now features a comprehensive dynamic image management system:

### Features:
- **Unlimited image slots** - Add as many images as you need
- **File browser integration** - Easy image selection with Windows file dialog
- **Multiple image formats** - Supports PNG, JPG, BMP, GIF, TIFF, and more
- **Real-time loading** - Load and reload images without restarting
- **Flexible display sizes** - Small (200x200), Medium (400x400), Large (600x600)
- **Path management** - Set custom paths for each image slot
- **Live preview** - See images immediately after loading

### How to Use:
1. Press **F2** to show the overlay
2. Navigate to the **"Dynamic Images"** tab
3. Click **"Add New Image Slot"** to create a new image slot
4. For each slot:
   - Type an image path or click **"Browse..."** to select a file
   - Click **"Load Image"** to load and display the image
   - Use the display size options to adjust how the image appears
   - Click **"Clear"** to remove the image and reset the slot
5. Use **"Remove Last Image"** to delete the most recent image slot

## Technologies Used

- **C++20** - Core programming language with modern features
- **Dear ImGui** - Immediate mode GUI library for user interface
- **DirectX 11** - Graphics API for hardware-accelerated rendering
- **Win32 API** - Windows application framework and system integration
- **WIC (Windows Imaging Component)** - Image loading and format support

## Building

This project is configured for Visual Studio 2022 with C++20 support.

1. Open `External_Overlay.sln` in Visual Studio
2. Select your preferred configuration (Debug/Release)
3. Build the solution (Ctrl+Shift+B)

## Requirements

- **Windows 10/11** - Modern Windows operating system
- **Visual Studio 2022** - With C++20 support enabled
- **DirectX 11 compatible graphics hardware** - For hardware acceleration
- **Administrator privileges** - Required for memory manipulation features

## Usage

### Basic Operation:
1. **Run the application** - Start the executable (preferably as administrator)
2. **Show overlay** - Press **F2** to make the overlay visible
3. **Navigate interface** - Use the tabs to access different features
4. **Toggle modes** - Press **F1** to switch between interactive and click-through modes
5. **Hide overlay** - Press **F2** again to hide the overlay

### Dynamic Image Management:
1. Go to **"Dynamic Images"** tab for the new image system
2. Add image slots and browse for your custom images
3. Load images in real-time and adjust display settings
4. Manage multiple images simultaneously

### Memory Editing:
1. Go to **"Memory Editor"** tab for game memory manipulation
2. Connect to target processes (like Elden Ring)
3. Read and modify memory values
4. Use preset actions or custom value inputs

### Legacy Images:
- The **"Legacy Images"** tab contains the original hardcoded image system
- This is kept for backward compatibility but **"Dynamic Images"** is recommended

## Supported Image Formats

- **JPEG/JPG** - Standard photo format
- **PNG** - With transparency support  
- **BMP** - Windows bitmap format
- **GIF** - Animated GIF (first frame only)
- **TIFF/TIF** - Tagged Image File Format
- All formats supported by Windows Imaging Component (WIC)

## Notes

- The overlay starts **hidden by default** - press F2 to show it
- Images are loaded using Windows Imaging Component for broad format support
- Memory manipulation features require administrator privileges
- The overlay maintains transparency and can be positioned over any application
- All dynamic images are managed in real-time without requiring application restart

## License

This project is for educational/personal use.
