#include<windows.h>
#include<dwmapi.h>
#include<d3d11.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>

 extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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

int APIENTRY WinMain(HINSTANCE instence, HINSTANCE, PSTR, INT cmd_show) {//instence of current application, previous instence, String pointer, show screen or not
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
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"Eternal overlay class";
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	
	if (!RegisterClassExW(&wc)) { // Check if registration was successful
		MessageBoxW(nullptr, L"Failed to register window class.", L"Error", MB_ICONERROR);
		return 1;
	}

	const HWND window = CreateWindowExW(
		WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED, //extended window styles
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
		MessageBoxW(nullptr, L"Failed to create window.", L"Error", MB_ICONERROR);
		return 1;
	}

	SetLayeredWindowAttributes(window, RGB(0, 0, 0), BYTE(255), LWA_ALPHA); //set window transparency

	{
		RECT client_area{};
		GetClientRect(window, &client_area);

		RECT window_area{};
		GetWindowRect(window, &window_area);

		POINT diff{};
		ClientToScreen(window, &diff);

		const MARGINS margins{
			window_area.left + (diff.x - window_area.left),
			window_area.top + (diff.y - window_area.top),
			client_area.right,
			client_area.bottom
		};
		DwmExtendFrameIntoClientArea(window, &margins);
	}

	// Fully initialize DXGI_SWAP_CHAIN_DESC
	DXGI_SWAP_CHAIN_DESC sd = {}; // Zero initialize first
	sd.BufferDesc.Width = 1280;
	sd.BufferDesc.Height = 720;
	sd.BufferDesc.RefreshRate.Numerator = 60U;
	sd.BufferDesc.RefreshRate.Denominator = 1U; //60 Hz refresh rate
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //32-bit color format
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

	constexpr D3D_FEATURE_LEVEL levels[2] = { //feature levels array
		D3D_FEATURE_LEVEL_11_0, //Direct3D 11.0
		D3D_FEATURE_LEVEL_10_0, //Direct3D 10.0
	};

	ID3D11Device* device = nullptr; //Direct3D device pointer
	ID3D11DeviceContext* device_context = nullptr; //Direct3D device context pointer
	IDXGISwapChain* swap_chain = nullptr; //swap chain pointer
	ID3D11RenderTargetView* render_target_view = nullptr; //render target view pointer
	D3D_FEATURE_LEVEL level{};

	// Add error handling for D3D11CreateDeviceAndSwapChain
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

	if (FAILED(hr) || !swap_chain) {
		wchar_t buf[128];
		wsprintf(buf, L"Failed to create D3D11 device and swap chain. HRESULT: 0x%08X", hr);
		MessageBoxW(window, buf, L"Error", MB_ICONERROR);
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
		ImGui_ImplDX11_NewFrame(); //start new ImGui frame for Direct3D 11
		ImGui_ImplWin32_NewFrame(); //start new ImGui frame for Win32

		ImGui::NewFrame(); //start new ImGui frame

		


		ImGui::Render();

		constexpr float color[4] = { 0.f, 0.f, 0.f, 0.f }; //clear color (black)
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