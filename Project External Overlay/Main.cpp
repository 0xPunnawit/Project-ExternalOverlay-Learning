#include <Windows.h>
#include <d3d11.h>
#include <tchar.h>
#include <dwmapi.h>
#include <iostream>

#include "./imgui/imgui.h"
#include "./imgui/imgui_impl_win32.h"
#include "./imgui/imgui_impl_dx11.h"

#include "process.h"

#include "./gui/gui.h"


using namespace std;

GUI gui;

LRESULT WINAPI WindowsProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	PWSTR pCmdLine,
	int nCmdShow
)
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = WindowsProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;

	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = L"ExternalOverlay Class";
	wc.lpszMenuName = NULL;
	wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);
	wc.hCursor = LoadCursor(hInstance, IDC_ARROW);

	RegisterClassEx(&wc);

	int width, height;
	HWND targetWindow = FindWindowA(0, "AssaultCube");
	if (targetWindow)
	{
		RECT rect;
		GetWindowRect(targetWindow, &rect);
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}
	else
	{
		return 0;
	}

	HWND hWnd = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
		L"ExternalOverlay Class",
		L"ExtrernalOverlay",
		WS_POPUP,
		0,
		0,
		width,
		height,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

	RECT client_area{};
	GetClientRect(hWnd, &client_area);

	RECT windows_area{};
	GetWindowRect(hWnd, &windows_area);

	POINT diff{};
	ClientToScreen(hWnd, &diff);

	const MARGINS margins
	{
		windows_area.left + (diff.x - windows_area.left),
		windows_area.top + (diff.y - windows_area.top),
		client_area.right,
		client_area.bottom
	};

	DwmExtendFrameIntoClientArea(hWnd, &margins);


	// Initialize Direct3D
	if (!gui.CreateDeviceD3D(hWnd))
	{
		gui.CleanupDeviceD3D();
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(gui.g_pd3dDevice, gui.g_pd3dDeviceContext);


	// Our state
	bool show_menu = false;

	bool cGodmode = false;
	bool cArmor = false;
	bool cGrenade = false;

	ImVec4 clear_color = ImVec4(0.f, 0.f, 0.f, 0.f);						// layered windows ให้เป็นค่าใส

	// Main loop
	bool done = false;
	while (!done)
	{

		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);

			RECT rect;
			GetWindowRect(targetWindow, &rect);
			width = rect.right - rect.left;
			height = rect.bottom - rect.top;

			MoveWindow(hWnd, rect.left, rect.top, width, height, true);

			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;

		// Handle window resize (we don't resize directly in the WM_SIZE handler)
		if (gui.g_ResizeWidth != 0 && gui.g_ResizeHeight != 0)
		{
			gui.CleanupRenderTarget();
			gui.g_pSwapChain->ResizeBuffers(0, gui.g_ResizeWidth, gui.g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
			gui.g_ResizeWidth = gui.g_ResizeHeight = 0;
			gui.CreateRenderTarget();
		}

		// Start the Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if (GetAsyncKeyState(VK_DELETE) & 1)
		{
			show_menu = !show_menu;
		}

		long style = GetWindowLong(hWnd, GWL_EXSTYLE);

		if (show_menu)												// เช็คว่า ว่า TRUE ป่าว
		{
			style &= ~WS_EX_LAYERED;								// ~ เปลี่ยนค่า บิต จาก 1 เป็น 0 เหมือนลบ
			SetWindowLong(hWnd, GWL_EXSTYLE, style);
		}
		else
		{
			style |= WS_EX_LAYERED;									// | เปลี่ยนค่าจาก 0 เป็น 1 เหมือน เพิ่ม 
			SetWindowLong(hWnd, GWL_EXSTYLE, style);
		}

		if (show_menu)
		{
			gui.RenderMenu();
		}

		// Star Hack

		gui.Hack();

		// End Hack

		// Rendering
		ImGui::Render();
		const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
		gui.g_pd3dDeviceContext->OMSetRenderTargets(1, &gui.g_mainRenderTargetView, nullptr);
		gui.g_pd3dDeviceContext->ClearRenderTargetView(gui.g_mainRenderTargetView, clear_color_with_alpha);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// Present
		HRESULT hr = gui.g_pSwapChain->Present(1, 0);   // Present with vsync
		//HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
		gui.g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
	}

	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	gui.CleanupDeviceD3D();
	::DestroyWindow(hWnd);
	::UnregisterClassW(wc.lpszClassName, wc.hInstance);

	return 0;
}


// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowsProc(
	HWND hWnd,
	UINT Msg,
	WPARAM wParam,
	LPARAM lParam
)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
		return true;
	switch (Msg)
	{
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
			return 0;
		gui.g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
		gui.g_ResizeHeight = (UINT)HIWORD(lParam);
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProcW(hWnd, Msg, wParam, lParam);
}