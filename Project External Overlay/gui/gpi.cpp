#include "gui.h"
#include "../process.h"

GUI::GUI()
{
}

GUI::~GUI()
{
}

// Render Direct3D 11 ImGui
bool GUI::CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void GUI::CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void GUI::CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void GUI::CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Attribute CheckBox
bool cGodmode = false;
bool cArmor = false;
bool cGrenade = false;

uintptr_t processId = GetProcessID(L"ac_client.exe");								// หาค่า PID game 
uintptr_t moduleBaseAddress = GetModuleBaseAddress(processId, L"ac_client.exe");	// หาค่า ModuleBase game
HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, processId);					// หาค่า ที่อยู่ในหน่วยความจำ 

// Render Menu Gui
void GUI::RenderMenu()
{
	int n = 0;

	ImGui::Begin("0xPunnawit"); 
	ImGui::Text("This is some useful text."); 
	ImGui::SliderInt("testSlider", &n, 0, 10); 

	if (ImGui::Button("Death"))
	{
		static int health = 0;
		uintptr_t health_address = FindDMAAddy(hProcess, moduleBaseAddress + 0x0017E0A8, { 0xEC });
		WriteProcessMemory(hProcess, (BYTE*)health_address, &health, sizeof(health), nullptr);
	}

	ImGui::Checkbox("God Mode", &cGodmode);
	ImGui::Checkbox("Arrmor", &cArmor);
	ImGui::Checkbox("Grenade", &cGrenade);

	ImGui::End();
}

// Hack Game Address
void GUI::Hack()
{

	if (cGodmode)
	{
		int health = 999;
		uintptr_t health_address = FindDMAAddy(hProcess, moduleBaseAddress + 0x0017E0A8, { 0xEC });
		WriteProcessMemory(hProcess, (BYTE*)health_address, &health, sizeof(health), nullptr);
	}

	if (cArmor)
	{
		int Armor = 999;
		uintptr_t Armor_address = FindDMAAddy(hProcess, moduleBaseAddress + 0x0017E0A8, { 0xF0 });
		WriteProcessMemory(hProcess, (BYTE*)Armor_address, &Armor, sizeof(Armor), nullptr);
	}

	if (cGrenade)
	{
		int Grenade = 999;
		uintptr_t Grenade_address = FindDMAAddy(hProcess, moduleBaseAddress + 0x0017E0A8, { 0x144 });
		WriteProcessMemory(hProcess, (BYTE*)Grenade_address, &Grenade, sizeof(Grenade), nullptr);
	}
}