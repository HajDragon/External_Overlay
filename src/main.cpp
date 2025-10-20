#include<windows.h>
#include<dwmapi.h>
#include<d3d11.h>
#include<cstring>
#include<string>
#include<chrono>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>

#include "ImageTexture.h" // Include our custom image texture loader
#include "ProcessManager.h" // Include our process manager
#include "MemoryManager.h" // Include our memory manager

 extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Helper function to display an image tab
void DisplayImageTab(DX11ImageTexture& image, const wchar_t* imagePath, bool& isLoaded, ID3D11Device* device) {
	// Convert wide string to multibyte for ImGui display
	char pathA[512];
	WideCharToMultiByte(CP_UTF8, 0, imagePath, -1, pathA, sizeof(pathA), nullptr, nullptr);
	
	ImGui::Text("Image Path:");
	ImGui::TextWrapped("%s", pathA);
	
	if (image.IsValid() && isLoaded) {
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "✓ Image loaded successfully");
		ImGui::Text("Dimensions: %dx%d", image.width, image.height);
		
		ImGui::Separator();
		
		// Display the image with multiple size options
		ImGui::Text("Display Options:");
		
		static int displayMode = 0;
		ImGui::RadioButton("Small (256x256)", &displayMode, 0); ImGui::SameLine();
		ImGui::RadioButton("Medium (400x400)", &displayMode, 1); ImGui::SameLine();
		ImGui::RadioButton("Large (600x600)", &displayMode, 2);
		ImGui::RadioButton("Original Size", &displayMode, 3); ImGui::SameLine();
		ImGui::RadioButton("Fit Window", &displayMode, 4);
		
		ImVec2 imageSize;
		switch (displayMode) {
			case 0: imageSize = ImVec2(256, 256); break;
			case 1: imageSize = ImVec2(400, 400); break;
			case 2: imageSize = ImVec2(600, 600); break;
			case 3: imageSize = ImVec2((float)image.width, (float)image.height); break;
			case 4: {
				ImVec2 windowSize = ImGui::GetWindowSize();
				float aspectRatio = (float)image.width / (float)image.height;
				imageSize = ImVec2(windowSize.x - 40, (windowSize.x - 40) / aspectRatio);
				break;
			}
		}
		
		ImGui::Separator();
		ImGui::Text("Displaying image:");
		
		// Add a border around the image
		ImVec2 cursorPos = ImGui::GetCursorScreenPos();
		ImGui::GetWindowDrawList()->AddRect(
			cursorPos, 
			ImVec2(cursorPos.x + imageSize.x, cursorPos.y + imageSize.y), 
			IM_COL32(255, 255, 255, 100), 
			0.0f, 
			0, 
			2.0f
		);
		
		// Display the actual image
		ImGui::Image(image.GetImGuiTextureID(), imageSize);
		
		// Add image interaction
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Click to cycle through sizes\nRight-click for context menu");
			if (ImGui::IsItemClicked()) {
				displayMode = (displayMode + 1) % 5;
			}
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
				ImGui::OpenPopup("ImageContextMenu");
			}
		}
		
		// Context menu
		if (ImGui::BeginPopup("ImageContextMenu")) {
			if (ImGui::MenuItem("Small View")) displayMode = 0;
			if (ImGui::MenuItem("Medium View")) displayMode = 1;
			if (ImGui::MenuItem("Large View")) displayMode = 2;
			if (ImGui::MenuItem("Original Size")) displayMode = 3;
			if (ImGui::MenuItem("Fit Window")) displayMode = 4;
			ImGui::Separator();
			if (ImGui::MenuItem("Reload Image")) {
				isLoaded = image.LoadFromFile(device, imagePath);
			}
			ImGui::EndPopup();
		}
		
	} else {
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "✗ Failed to load image");
		ImGui::Separator();
		ImGui::Text("Possible issues:");
		ImGui::BulletText("File doesn't exist at the specified path");
		ImGui::BulletText("File format not supported");
		ImGui::BulletText("File is corrupted or incomplete");
		ImGui::BulletText("Insufficient memory to load image");
		
		if (ImGui::Button("Retry Loading")) {
			isLoaded = image.LoadFromFile(device, imagePath);
		}
	}
}

// Helper function to display memory editor tab
void DisplayMemoryEditorTab(ProcessManager& processManager, MemoryManager& memoryManager) {
    // Process connection section
    ImGui::Text("Process Connection:");
    
    // Static variables to maintain state between frames
    static char processName[256] = "eldenring.exe";
    static char valueStr[32] = "0";
    static bool isConnected = false;
    static std::uint32_t currentRuneValue = 0;
    static std::uint32_t newRuneValue = 0;
    
    // Address selection options
    static bool useCustomAddress = false;
    static char customAddressStr[32] = "7FF3F0A3AC6C";
    static constexpr uintptr_t defaultRuneAddress = 0x7FF3F0A3AC6CULL;
    static uintptr_t customRuneAddress = defaultRuneAddress;
    
    // Parse custom address if using custom mode
    if (useCustomAddress) {
        customRuneAddress = std::strtoull(customAddressStr, nullptr, 16);
    }
    
    // Current working address
    uintptr_t currentRuneAddress = useCustomAddress ? customRuneAddress : defaultRuneAddress;
    
    // Connection status display
    if (isConnected && processManager.IsAttached() && processManager.IsProcessRunning()) {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "✓ Connected to process");
        ImGui::Text("Process: %s", processName);
        ImGui::Text("Process ID: %lu", processManager.GetProcessId());
    } else {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "✗ Not connected");
        isConnected = false;
    }
    
    ImGui::Separator();
    
    // Process connection controls
    ImGui::Text("Target Process:");
    ImGui::InputText("Process Name", processName, sizeof(processName));
    
    if (ImGui::Button("Connect to Process")) {
        std::wstring wProcessName(processName, processName + strlen(processName));
        if (processManager.AttachToProcess(wProcessName)) {
            isConnected = true;
        } else {
            isConnected = false;
        }
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Disconnect")) {
        processManager.DetachFromProcess();
        isConnected = false;
    }
    
    ImGui::Separator();
    
    // Address selection section
    ImGui::Text("Memory Address Configuration:");
    
    // Toggle between default and custom address
    if (ImGui::RadioButton("Use Default Address", !useCustomAddress)) {
        useCustomAddress = false;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Use Custom Address", useCustomAddress)) {
        useCustomAddress = true;
    }
    
    if (useCustomAddress) {
        ImGui::Text("Custom Address (Hex):");
        ImGui::InputText("##CustomAddress", customAddressStr, sizeof(customAddressStr), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
        ImGui::SameLine();
        if (ImGui::Button("Apply")) {
            customRuneAddress = std::strtoull(customAddressStr, nullptr, 16);
        }
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Current Custom Address: 0x%llX", customRuneAddress);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "Enter address without '0x' prefix (e.g., 7FF3F0A3AC6C)");
    } else {
        ImGui::TextColored(ImVec4(0, 1, 1, 1), "Using Default Address: 0x%llX", defaultRuneAddress);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "This is the verified Cheat Engine address");
    }
    
    ImGui::Separator();
    
    // Display current working address
    ImGui::Text("Active Rune Address: 0x%llX", currentRuneAddress);
    if (useCustomAddress) {
        ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "⚠ Using custom address - verify this is correct!");
    } else {
        ImGui::TextColored(ImVec4(0, 1, 1, 1), "✓ Using verified default address");
    }
    
    ImGui::Separator();
    
    // Memory manipulation section
    ImGui::Text("Rune Memory Editor:");
    
    if (isConnected && processManager.IsAttached()) {
        // Validate address
        if (memoryManager.IsValidAddress(currentRuneAddress)) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "✓ Address is valid and accessible");
        } else {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "⚠ Address validation failed");
            if (useCustomAddress) {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Check if your custom address is correct!");
            }
        }
        
        ImGui::Separator();
        
        // Read current value
        if (ImGui::Button("Read Current Runes")) {
            auto result = memoryManager.ReadUInt32(currentRuneAddress);
            if (result.has_value()) {
                currentRuneValue = result.value();
                sprintf_s(valueStr, "%u", currentRuneValue);
            } else {
                ImGui::OpenPopup("ReadErrorPopup");
            }
        }
        
        ImGui::Text("Current Rune Value: %u", currentRuneValue);
        
        // Auto-refresh option
        static bool autoRefresh = false;
        ImGui::Checkbox("Auto-refresh every second", &autoRefresh);
        
        if (autoRefresh) {
            static auto lastRefresh = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - lastRefresh).count() >= 1) {
                auto result = memoryManager.ReadUInt32(currentRuneAddress);
                if (result.has_value()) {
                    currentRuneValue = result.value();
                    sprintf_s(valueStr, "%u", currentRuneValue);
                }
                lastRefresh = now;
            }
        }
        
        ImGui::Separator();
        
        // Write new value section
        ImGui::Text("Set New Rune Value:");
        ImGui::InputText("New Value", valueStr, sizeof(valueStr), ImGuiInputTextFlags_CharsDecimal);
        
        // Parse the input value
        newRuneValue = static_cast<std::uint32_t>(std::strtoul(valueStr, nullptr, 10));
        ImGui::Text("Value to write: %u", newRuneValue);
        
        if (ImGui::Button("Write Rune Value")) {
            if (memoryManager.WriteUInt32(currentRuneAddress, newRuneValue)) {
                ImGui::OpenPopup("SuccessPopup");
                currentRuneValue = newRuneValue; // Update displayed current value
            } else {
                ImGui::OpenPopup("ErrorPopup");
            }
        }
        
        // Read Error popup
        if (ImGui::BeginPopupModal("ReadErrorPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "✗ Failed to read memory!");
            ImGui::Text("Possible causes:");
            ImGui::BulletText("Invalid memory address");
            ImGui::BulletText("Process memory protection");
            ImGui::BulletText("Address is not accessible");
            if (useCustomAddress) {
                ImGui::Separator();
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "💡 Your custom address might be incorrect");
                ImGui::Text("Try using the default address or find the correct");
                ImGui::Text("address using Cheat Engine");
            }
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        
        // Success popup
        if (ImGui::BeginPopupModal("SuccessPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "✓ Rune value updated successfully!");
            ImGui::Text("New value: %u", newRuneValue);
            ImGui::Text("Address used: 0x%llX", currentRuneAddress);
            ImGui::Text("Check Elden Ring to confirm the change!");
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        
        // Error popup
        if (ImGui::BeginPopupModal("ErrorPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "✗ Failed to write memory!");
            ImGui::Text("Address used: 0x%llX", currentRuneAddress);
            ImGui::Separator();
            ImGui::Text("Possible causes:");
            ImGui::BulletText("Process is not running");
            ImGui::BulletText("Insufficient permissions (run as administrator)");
            ImGui::BulletText("Memory address has changed");
            ImGui::BulletText("Memory protection preventing write");
            ImGui::BulletText("Game update changed the address");
            if (useCustomAddress) {
                ImGui::BulletText("Custom address is incorrect");
            }
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "💡 Troubleshooting tips:");
            if (useCustomAddress) {
                ImGui::Text("• Try using the default address first");
                ImGui::Text("• Verify your custom address with Cheat Engine");
            } else {
                ImGui::Text("• Try finding new address with Cheat Engine");
                ImGui::Text("• Make sure you're running as administrator");
            }
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        
        ImGui::Separator();
        
        // Quick value buttons
        ImGui::Text("Quick Actions:");
        if (ImGui::Button("Add 1000 Runes")) {
            auto currentOpt = memoryManager.ReadUInt32(currentRuneAddress);
            if (currentOpt.has_value()) {
                std::uint32_t newVal = currentOpt.value() + 1000;
                if (memoryManager.WriteUInt32(currentRuneAddress, newVal)) {
                    currentRuneValue = newVal;
                    sprintf_s(valueStr, "%u", newVal);
                }
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Add 10000 Runes")) {
            auto currentOpt = memoryManager.ReadUInt32(currentRuneAddress);
            if (currentOpt.has_value()) {
                std::uint32_t newVal = currentOpt.value() + 10000;
                if (memoryManager.WriteUInt32(currentRuneAddress, newVal)) {
                    currentRuneValue = newVal;
                    sprintf_s(valueStr, "%u", newVal);
                }
            }
        }
        
        if (ImGui::Button("Add 100000 Runes")) {
            auto currentOpt = memoryManager.ReadUInt32(currentRuneAddress);
            if (currentOpt.has_value()) {
                std::uint32_t newVal = currentOpt.value() + 100000;
                if (memoryManager.WriteUInt32(currentRuneAddress, newVal)) {
                    currentRuneValue = newVal;
                    sprintf_s(valueStr, "%u", newVal);
                }
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Set Max Runes (999999999)")) {
            constexpr std::uint32_t maxRunes = 999999999;
            if (memoryManager.WriteUInt32(currentRuneAddress, maxRunes)) {
                currentRuneValue = maxRunes;
                sprintf_s(valueStr, "%u", maxRunes);
            }
        }
        
        ImGui::Separator();
        
        // Custom value input section
        ImGui::Text("Custom Actions:");
        static char customAmountStr[32] = "50000";
        ImGui::InputText("Custom Amount", customAmountStr, sizeof(customAmountStr), ImGuiInputTextFlags_CharsDecimal);
        
        std::uint32_t customAmount = static_cast<std::uint32_t>(std::strtoul(customAmountStr, nullptr, 10));
        ImGui::Text("Amount: %u", customAmount);
        
        if (ImGui::Button("Add Custom Amount")) {
            auto currentOpt = memoryManager.ReadUInt32(currentRuneAddress);
            if (currentOpt.has_value()) {
                std::uint32_t newVal = currentOpt.value() + customAmount;
                if (memoryManager.WriteUInt32(currentRuneAddress, newVal)) {
                    currentRuneValue = newVal;
                    sprintf_s(valueStr, "%u", newVal);
                }
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Subtract Custom Amount")) {
            auto currentOpt = memoryManager.ReadUInt32(currentRuneAddress);
            if (currentOpt.has_value()) {
                std::uint32_t currentVal = currentOpt.value();
                std::uint32_t newVal = (currentVal > customAmount) ? (currentVal - customAmount) : 0;
                if (memoryManager.WriteUInt32(currentRuneAddress, newVal)) {
                    currentRuneValue = newVal;
                    sprintf_s(valueStr, "%u", newVal);
                }
            }
        }
        
        ImGui::Separator();
        
        // Address info section
        ImGui::Text("Address Information:");
        if (useCustomAddress) {
            ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Using custom address - make sure it's correct!");
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "Use Cheat Engine to find valid addresses.");
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "Switch to default address if having issues.");
        } else {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "Default address was found using Cheat Engine.");
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "If it stops working, try custom address mode.");
        }
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Note: Addresses may change between game sessions!");
        
    } else {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "Connect to Elden Ring process first to edit runes");
        ImGui::Separator();
        ImGui::Text("Instructions:");
        ImGui::BulletText("Launch Elden Ring");
        ImGui::BulletText("Choose default or custom address");
        ImGui::BulletText("Click 'Connect to Process'");
        ImGui::BulletText("Use the buttons to modify your runes");
        ImGui::BulletText("Check the game to see changes");
    }
}

LRESULT CALLBACK window_Procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_Param) { //window procedure function
	if (ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_Param)) {

		return 0L;
	}

	if(message == WM_DESTROY) { //if message is destroy window
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(window, message, w_param, l_Param); //default window procedure function
}

int APIENTRY WinMain(_In_ HINSTANCE instence, _In_opt_ HINSTANCE, _In_ PSTR, _In_ INT cmd_show) { // instance of current application, previous instance, String pointer, show screen or not
	// Properly initialize the WNDCLASSEXW structure
	WNDCLASSEXW wc = {}; // Zero initialize all fields first
	wc.cbSize = sizeof(WNDCLASSEX); //size of structure
	wc.style = CS_HREDRAW | CS_VREDRAW; //redraw on horizontal or vertical resize
	wc.lpfnWndProc = window_Procedure; //pointer to window procedure function
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = instence; //handle to application instance
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	// Remove background brush to allow transparency
	wc.hbrBackground = nullptr; // No background brush for transparent overlay
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"Eternal overlay class";
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	
	if (!RegisterClassExW(&wc)) { // Check if registration was successful
		MessageBoxW(nullptr, L"Failed to register window class.", L"Error", MB_ICONERROR);
		return 1;
	}

	const HWND window = CreateWindowExW(
		WS_EX_TOPMOST | WS_EX_LAYERED, //extended window styles - removed WS_EX_TRANSPARENT to allow interaction
		wc.lpszClassName, //window class name
		L"Eternal overlay", //window title
		WS_POPUP, //window style (no border, no title bar, etc.)
		0, 0, 1280, 720, //x, y, width, height
		nullptr, //handle to parent window
		nullptr, //handle to menu
		wc.hInstance, //handle to application instance
		nullptr //pointer to window creation data
	);

	if (!window) {
		DWORD error = GetLastError();
		wchar_t buf[256];
		wsprintf(buf, L"Failed to create window. Error code: %lu", error);
		MessageBoxW(nullptr, buf, L"Window Creation Error", MB_ICONERROR);
		return 1;
	}

	// Remove the debugging window modifications and restore proper overlay settings
	SetLayeredWindowAttributes(window, RGB(0, 0, 0), BYTE(0), LWA_COLORKEY); //make black pixels transparent

	// Enable DWM composition for proper overlay transparency
	{
		RECT client_area{};
		GetClientRect(window, &client_area);

		// Extend the frame into the entire client area for full transparency support
		const MARGINS margins{
			-1, -1, -1, -1  // Extend frame into entire window
		};
		DwmExtendFrameIntoClientArea(window, &margins);
	}

	// Fully initialize DXGI_SWAP_CHAIN_DESC
	DXGI_SWAP_CHAIN_DESC sd = {}; // Zero initialize first
	sd.BufferDesc.Width = 1280;
	sd.BufferDesc.Height = 720;
	sd.BufferDesc.RefreshRate.Numerator = 60U;
	sd.BufferDesc.RefreshRate.Denominator = 1U; //60 Hz refresh rate
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; //32-bit color format with alpha support
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = 1U; //no multi-sampling
	sd.SampleDesc.Quality = 0U;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //render target output usage
	sd.BufferCount = 2U; //double buffering
	sd.OutputWindow = window; //handle to window
	sd.Windowed = TRUE; //windowed mode
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; //discard old frames
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Validate window handle before proceeding
	if (!IsWindow(window)) {
		MessageBoxW(nullptr, L"Invalid window handle for DirectX initialization.", L"Error", MB_ICONERROR);
		return 1;
	}

	constexpr D3D_FEATURE_LEVEL levels[2] = { //feature levels array
		D3D_FEATURE_LEVEL_11_0, //Direct3D 11.0
		D3D_FEATURE_LEVEL_10_0, //Direct3D 10.0
	};

	ID3D11Device* device = nullptr; //Direct3D device pointer
	ID3D11DeviceContext* device_context = nullptr; //Direct3D device context pointer
	IDXGISwapChain* swap_chain = nullptr; //swap chain pointer
	ID3D11RenderTargetView* render_target_view = nullptr; //render target view pointer
	D3D_FEATURE_LEVEL level{};

	// First, try creating device and swap chain together
	// Add error handling for D3D11CreateDeviceAndSwapChain with fallback options
	HRESULT hr = D3D11CreateDeviceAndSwapChain( //create Direct3D device and swap chain
		nullptr, //use default adapter
		D3D_DRIVER_TYPE_HARDWARE, //use hardware driver
		nullptr, //no software rasterizer
		0U, //no creation flags
		levels, //feature levels array
		2U, //number of feature levels
		D3D11_SDK_VERSION, //Direct3D SDK version
		&sd, //swap chain description structure
		&swap_chain, //pointer to swap chain pointer
		&device, //pointer to device pointer
		&level, //pointer to feature level variable
		&device_context //pointer to device context pointer
	);

	// If hardware driver fails, try WARP (software) driver
	if (FAILED(hr)) {
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_WARP, // Use WARP software driver as fallback
			nullptr,
			0U,
			levels,
			2U,
			D3D11_SDK_VERSION,
			&sd,
			&swap_chain,
			&device,
			&level,
			&device_context
		);
	}

	// If still failing, try with different swap chain settings
	if (FAILED(hr)) {
		// Try with single buffering and different format
		sd.BufferCount = 1U; // Single buffering
		sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // Different format
		sd.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL; // Sequential swap effect
		
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			0U,
			levels,
			2U,
			D3D11_SDK_VERSION,
			&sd,
			&swap_chain,
			&device,
			&level,
			&device_context
		);
	}

	// Last resort: try creating device first, then swap chain separately
	if (FAILED(hr)) {
		// Try creating just the device first
		hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			0U,
			levels,
			2U,
			D3D11_SDK_VERSION,
			&device,
			&level,
			&device_context
		);

		if (SUCCEEDED(hr) && device) {
			// Now try creating swap chain with the device
			IDXGIDevice* dxgiDevice = nullptr;
			IDXGIAdapter* dxgiAdapter = nullptr;
			IDXGIFactory* dxgiFactory = nullptr;

			hr = device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
			if (SUCCEEDED(hr)) {
				hr = dxgiDevice->GetAdapter(&dxgiAdapter);
				if (SUCCEEDED(hr)) {
					hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
					if (SUCCEEDED(hr)) {
						hr = dxgiFactory->CreateSwapChain(device, &sd, &swap_chain);
					}
					dxgiAdapter->Release();
				}
				dxgiDevice->Release();
			}
			if (dxgiFactory) dxgiFactory->Release();
		}
	}

	if (FAILED(hr) || !swap_chain) {
		wchar_t buf[256];
		const wchar_t* errorMsg = L"Unknown error";
		
		// Provide more specific error messages
		switch (hr) {
			case DXGI_ERROR_UNSUPPORTED:
				errorMsg = L"DXGI_ERROR_UNSUPPORTED - The requested functionality is not supported";
				break;
			case DXGI_ERROR_INVALID_CALL:
				errorMsg = L"DXGI_ERROR_INVALID_CALL - Invalid parameters";
				break;
			case DXGI_ERROR_DEVICE_HUNG:
				errorMsg = L"DXGI_ERROR_DEVICE_HUNG - Device hung";
				break;
			case DXGI_ERROR_DEVICE_REMOVED:
				errorMsg = L"DXGI_ERROR_DEVICE_REMOVED - Device removed";
				break;
			case DXGI_ERROR_DEVICE_RESET:
				errorMsg = L"DXGI_ERROR_DEVICE_RESET - Device reset";
				break;
			case E_OUTOFMEMORY:
				errorMsg = L"E_OUTOFMEMORY - Out of memory";
				break;
			case E_INVALIDARG:
				errorMsg = L"E_INVALIDARG - Invalid argument";
				break;
		}
		
		wsprintf(buf, L"Failed to create D3D11 device and swap chain.\nHRESULT: 0x%08X\nError: %s\n\nTry updating your graphics drivers.", hr, errorMsg);
		MessageBoxW(window, buf, L"DirectX 11 Error", MB_ICONERROR);
		return 1;
	}

	ID3D11Texture2D* back_buffer = nullptr; //back buffer texture pointer
	hr = swap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer)); //get back buffer texture

	if (FAILED(hr) || !back_buffer) {
		MessageBoxW(window, L"Failed to get back buffer texture.", L"Error", MB_ICONERROR);
		return 1;
	}

	hr = device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view); //create render target view
	back_buffer->Release(); //release back buffer texture pointer

	if (FAILED(hr) || !render_target_view) {
		MessageBoxW(window, L"Failed to create render target view.", L"Error", MB_ICONERROR);
		return 1;
	}

	ShowWindow(window, cmd_show);
	UpdateWindow(window);

	ImGui::CreateContext(); //create ImGui context
	ImGui::StyleColorsDark(); //set ImGui style to dark

	ImGui_ImplWin32_Init(window); //initialize ImGui for Win32
	ImGui_ImplDX11_Init(device, device_context); //initialize ImGui for Direct3D 11

	// Initialize memory management system
	static ProcessManager processManager;
	static MemoryManager memoryManager(processManager);

	// Load an example image with better error handling
	static DX11ImageTexture birdImage;
	bool imageLoadSuccess = birdImage.LoadFromFile(device, L"C:\\Users\\Arshia\\Downloads\\bird ballsjpg.jpg");

	constexpr uint8_t maxImages = 4; // set the numebr of max images

	// Additional images for testing (add more if you have them)
	static DX11ImageTexture testImages[maxImages]; // Changed from [3] to [4]
	static const wchar_t* imagePaths[maxImages] = { // Changed from [3] to [4]
		L"C:\\Users\\Arshia\\Downloads\\bird ballsjpg.jpg",
		L"C:\\Windows\\Web\\Wallpaper\\Windows\\img0.jpg",
		L"C:\\Users\\Arshia\\Downloads\\ap24195803307155.jpg",
		L"C:\\Users\\Arshia\\Downloads\\pony.png",
	};
	static bool imagesLoaded[maxImages] = { false, false, false, false }; 
	// Try loading the bird image
	imagesLoaded[0] = birdImage.LoadFromFile(device, imagePaths[0]);
	
	// Try loading additional test images
	for (int i = 1; i < maxImages; i++) { // Changed from < 3 to < 4
		imagesLoaded[i] = testImages[i].LoadFromFile(device, imagePaths[i]);
	}
	
	// Debug info for image loading
	if (imageLoadSuccess) {
		// Image loaded successfully
	} else {
		// Image failed to load - we'll show an error message in the UI
	}

	// Overlay interaction mode control
	bool isClickThrough = false; // Start in interactive mode
	bool toggleKeyPressed = false;

	bool running = true;

	while (running) {
		MSG msg;
		while(PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) { //peek message loop
			TranslateMessage(&msg); //translate message
			DispatchMessage(&msg); //dispatch message to window procedure function
			if(msg.message == WM_QUIT) { //if message is quit
				running = false; //set running to false
			}
		}
		if(!running) { //if not running
			break; //break out of loop
		}

		 // Check for toggle hotkey (F1 key) to switch between interactive and click-through modes
		bool f1KeyDown = GetAsyncKeyState(VK_F1) & 0x8000;
		if (f1KeyDown && !toggleKeyPressed) {
			isClickThrough = !isClickThrough;
			
			// Update window properties based on mode
			LONG_PTR currentExStyle = GetWindowLongPtrW(window, GWL_EXSTYLE);
			if (isClickThrough) {
				// Make window click-through
				SetWindowLongPtrW(window, GWL_EXSTYLE, currentExStyle | WS_EX_TRANSPARENT);
			} else {
				// Make window interactive
				SetWindowLongPtrW(window, GWL_EXSTYLE, currentExStyle & ~WS_EX_TRANSPARENT);
			}
			
			toggleKeyPressed = true;
		} else if (!f1KeyDown) {
			toggleKeyPressed = false;
		}

		ImGui_ImplDX11_NewFrame(); //start new ImGui frame for Direct3D 11
		ImGui_ImplWin32_NewFrame(); //start new ImGui frame for Win32

		ImGui::NewFrame(); //start new ImGui frame

		// --- Start rendering your ImGui widgets or custom drawing here ---
		ImGui::Begin("Image Overlay", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		
		 // Display current overlay mode and instructions
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Overlay Mode:");
		if (isClickThrough) {
			ImGui::TextColored(ImVec4(0, 1, 1, 1), "🔄 Click-Through Mode");
			ImGui::Text("Mouse input passes through to applications below");
		} else {
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "🖱️ Interactive Mode");
			ImGui::Text("You can click and interact with this overlay");
		}
		ImGui::TextColored(ImVec4(1, 1, 1, 0.7f), "Press F1 to toggle between modes");
		ImGui::Separator();
		
		// Image selection tabs
		if (ImGui::BeginTabBar("MainTabs")) {
			// Memory Editor tab
			if (ImGui::BeginTabItem("Memory Editor")) {
				DisplayMemoryEditorTab(processManager, memoryManager);
				ImGui::EndTabItem();
			}
			
			// Bird image tab
			if (ImGui::BeginTabItem("Bird Image")) {
				DisplayImageTab(birdImage, imagePaths[0], imagesLoaded[0], device);
				ImGui::EndTabItem();
			}
			
			// Test images tabs
			for (int i = 1; i < maxImages; i++) {
				wchar_t tabName[64];
				wsprintf(tabName, L"Test Image %d", i);
				char tabNameA[64];
				WideCharToMultiByte(CP_UTF8, 0, tabName, -1, tabNameA, sizeof(tabNameA), nullptr, nullptr);
				
				if (ImGui::BeginTabItem(tabNameA)) {
					DisplayImageTab(testImages[i], imagePaths[i], imagesLoaded[i], device);
					ImGui::EndTabItem();
				}
			}
			
			ImGui::EndTabBar();
		}
		
		ImGui::End();
		
		// Memory editor tab
		ImGui::Begin("Memory Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		DisplayMemoryEditorTab(processManager, memoryManager);
		ImGui::End();
		
		// --- End of your rendering code ---

		ImGui::Render();

		// Clear with fully transparent black for overlay transparency
		constexpr float color[4] = { 0.f, 0.f, 0.f, 0.f }; //clear color (transparent black)
		device_context->OMSetRenderTargets(1U, &render_target_view, nullptr); //set render target
		device_context->ClearRenderTargetView(render_target_view, color); //clear render target view

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); //render ImGui draw data

		swap_chain->Present(1U, 0U); //present swap chain (vsync enabled)
	}

	ImGui_ImplDX11_Shutdown(); //shutdown ImGui for Direct3D 11
	ImGui_ImplWin32_Shutdown(); //shutdown ImGui for Win32

	ImGui::DestroyContext(); //destroy ImGui context

	if (swap_chain) {
		swap_chain->Release(); //release swap chain pointer
	}
	if(device_context) {
		device_context->Release(); //release device context pointer
	}
	if (device) {
		device->Release(); //release device pointer
	}
	if (render_target_view) {
		render_target_view->Release(); //release render target view pointer
	}

	DestroyWindow(window); //destroy window
	UnregisterClassW(wc.lpszClassName, wc.hInstance); //unregister window class structure

	return 0;
}