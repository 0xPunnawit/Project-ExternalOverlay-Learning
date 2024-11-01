#include "gui.h"
#include "../Memory.h"
#include "../Font/IconsFontAwesome6.h"

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


// Render Menu Gui
void GUI::RenderMenu()
{
	ImGui::Begin("0xPunnawit");

	ImGui::BeginChild("Sidebar", ImVec2(120, 150));

	ImVec2 buttonSize = ImVec2(100, 40);

	if (ImGui::Button(ICON_FA_CROSSHAIRS " Aimbot", buttonSize)) selectedSidebar = 1;
	if (ImGui::Button(ICON_FA_EYE " ESP", buttonSize)) selectedSidebar = 2;
	if (ImGui::Button(ICON_FA_OTTER " Other", buttonSize)) selectedSidebar = 3;

	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginGroup();

	if (selectedSidebar == 1)
	{
		ImGui::Text("Aimbot");
		ImGui::Checkbox("Enable", &b_aimbot);
		if (b_aimbot)
		{
			ImGui::Combo("Target", &target_current, target, IM_ARRAYSIZE(target));
		}

	}

	if (selectedSidebar == 2)
	{
		ImGui::Text("ESP");
		ImGui::Checkbox("2D BOX", &b_2d_box);
		ImGui::Checkbox("Line", &b_line);
		ImGui::Checkbox("Health", &b_2d_health);
		ImGui::Checkbox("Player Name", &b_name);
	}

	if (selectedSidebar == 3)
	{
		ImGui::Text("Other");
		ImGui::Checkbox("Godmode", &b_godmode);
		ImGui::SameLine();  // ทำให้ปุ่มถัดไปอยู่ในบรรทัดเดียวกัน
		ImGui::Checkbox("Armor", &b_armor);
		ImGui::SameLine();  // ทำให้ปุ่มถัดไปอยู่ในบรรทัดเดียวกัน
		ImGui::Checkbox("Grenade", &b_grenade);
	}


	ImGui::EndGroup();


	ImGui::End();
}

// Hack Game Address
void GUI::Hack()
{
	Memory mem = Memory(L"ac_client.exe", L"ac_client.exe");

	float view_matrix[16];
	ReadProcessMemory(mem.hProcess, (BYTE*)0x57DFD0, &view_matrix, sizeof(view_matrix), nullptr);

	if (b_godmode)
	{
		mem.WriteDMA(mem.moduleBaseAddress + 0x0017E0A8, { 0xEC }, 888);

	}

	if (b_armor)
	{
		mem.WriteDMA(mem.moduleBaseAddress + 0x0017E0A8, { 0xF0 }, 888);
	}

	if (b_grenade)
	{
		mem.WriteDMA(mem.moduleBaseAddress + 0x0017E0A8, { 0x144 }, 888);
	}

	Vector2 top;
	Vector2 bottom;

	uintptr_t entity = mem.moduleBaseAddress + 0x0018AC00;
	uintptr_t local_player = mem.Read<uintptr_t>(entity);
	uintptr_t entity_list = mem.Read<uintptr_t>(entity + 0x4) + 0x4;
	int num_of_player = mem.Read<int>(entity + 0xC) - 1;

	if (b_2d_box)
	{
		for (unsigned int i = 0; i < num_of_player; i++)
		{
			uintptr_t player = mem.Read<uintptr_t>(entity_list + 0x4 * i);

			if (player == NULL)
			{
				continue;
			}

			int health = mem.Read<int>(player + 0xEC);
			if (health <= 0)
			{
				continue;
			}

			Vector3 pos_head = {
				mem.Read<float>(player + 0x4),  // x
				mem.Read<float>(player + 0x8),  // y
				mem.Read<float>(player + 0xC)   // z
			};

			Vector3 pos_bottom = {
				mem.Read<float>(player + 0x28), // x
				mem.Read<float>(player + 0x2C), // y
				mem.Read<float>(player + 0x30)  // z
			};

			const float windows_width = mem.Read<int>(mem.moduleBaseAddress + 0x191ED8);
			const float windows_height = mem.Read<int>(mem.moduleBaseAddress + 0x191EDC);

			// เรียกใช้ WordToScreen สำหรับตำแหน่งหัวและฐาน
			if (WordToScreen(pos_head, top, view_matrix, windows_width, windows_height) &&
				WordToScreen(pos_bottom, bottom, view_matrix, windows_width, windows_height))
			{
				const float h = bottom.y - top.y;
				const float w = h * 0.25f;

				if (mem.Read<int>(local_player + 0x30C) == mem.Read<int>(player + 0x30C))
				{
					ImGui::GetBackgroundDrawList()->AddRect({ top.x - w, top.y }, { bottom.x + w, bottom.y }, ImColor(0, 255, 0), 0, 0, 1);
				}
				else
				{
					ImGui::GetBackgroundDrawList()->AddRect({ top.x - w, top.y }, { bottom.x + w, bottom.y }, ImColor(255, 0, 0), 0, 0, 1);
				}
			}
		}
	}

	if (b_line)
	{
		for (unsigned int i = 0; i < num_of_player; i++)
		{
			uintptr_t player = mem.Read<uintptr_t>(entity_list + 0x4 * i);

			if (player == NULL)
			{
				continue;
			}

			int health = mem.Read<int>(player + 0xEC);
			if (health <= 0)
			{
				continue;
			}

			Vector3 pos_head = {
				mem.Read<float>(player + 0x4),  // x
				mem.Read<float>(player + 0x8),  // y
				mem.Read<float>(player + 0xC)   // z
			};

			Vector3 pos_bottom = {
				mem.Read<float>(player + 0x28), // x
				mem.Read<float>(player + 0x2C), // y
				mem.Read<float>(player + 0x30)  // z
			};

			const float windows_width = mem.Read<int>(mem.moduleBaseAddress + 0x191ED8);
			const float windows_height = mem.Read<int>(mem.moduleBaseAddress + 0x191EDC);

			// เรียกใช้ WordToScreen สำหรับตำแหน่งหัวและฐาน
			if (WordToScreen(pos_head, top, view_matrix, windows_width, windows_height) &&
				WordToScreen(pos_bottom, bottom, view_matrix, windows_width, windows_height))
			{
				const float h = bottom.y - top.y;
				const float w = h * 0.25f;

				if (mem.Read<int>(local_player + 0x30C) == mem.Read<int>(player + 0x30C))
				{
					ImGui::GetBackgroundDrawList()->AddLine({ windows_width / 2, 30 }, { top.x, top.y }, ImColor(0, 255, 0), 1);
				}
				else
				{
					ImGui::GetBackgroundDrawList()->AddLine({ windows_width / 2, 30 }, { top.x, top.y }, ImColor(255, 0, 0), 1);
				}

			}
		}
	}

	if (b_2d_health)
	{
		for (unsigned int i = 0; i < num_of_player; i++)
		{
			uintptr_t player = mem.Read<uintptr_t>(entity_list + 0x4 * i);

			if (player == NULL)
			{
				continue;
			}

			int health = mem.Read<int>(player + 0xEC);
			if (health <= 0)
			{
				continue;
			}

			Vector3 pos_head = {
				mem.Read<float>(player + 0x4),  // x
				mem.Read<float>(player + 0x8),  // y
				mem.Read<float>(player + 0xC)   // z
			};

			Vector3 pos_bottom = {
				mem.Read<float>(player + 0x28), // x
				mem.Read<float>(player + 0x2C), // y
				mem.Read<float>(player + 0x30)  // z
			};

			const float windows_width = mem.Read<int>(mem.moduleBaseAddress + 0x191ED8);
			const float windows_height = mem.Read<int>(mem.moduleBaseAddress + 0x191EDC);

			// เรียกใช้ WordToScreen สำหรับตำแหน่งหัวและฐาน
			if (WordToScreen(pos_head, top, view_matrix, windows_width, windows_height) &&
				WordToScreen(pos_bottom, bottom, view_matrix, windows_width, windows_height))
			{
				const float h = bottom.y - top.y;
				const float w = h * 0.25f;

				DrawVerticalBar(top.x - w - 15, top.y, 10, h, health, 100);
			}
		}
	}

	if (b_name)
	{
		for (unsigned int i = 0; i < num_of_player; i++)
		{
			uintptr_t player = mem.Read<uintptr_t>(entity_list + 0x4 * i);

			if (player == NULL)
			{
				continue;
			}

			int health = mem.Read<int>(player + 0xEC);
			if (health <= 0)
			{
				continue;
			}

			Vector3 pos_head = {
				mem.Read<float>(player + 0x4),  // x
				mem.Read<float>(player + 0x8),  // y
				mem.Read<float>(player + 0xC)   // z
			};

			Vector3 pos_bottom = {
				mem.Read<float>(player + 0x28), // x
				mem.Read<float>(player + 0x2C), // y
				mem.Read<float>(player + 0x30)  // z
			};

			const float windows_width = mem.Read<int>(mem.moduleBaseAddress + 0x191ED8);
			const float windows_height = mem.Read<int>(mem.moduleBaseAddress + 0x191EDC);

			// เรียกใช้ WordToScreen สำหรับตำแหน่งหัวและฐาน
			if (WordToScreen(pos_head, top, view_matrix, windows_width, windows_height) &&
				WordToScreen(pos_bottom, bottom, view_matrix, windows_width, windows_height))
			{
				const float h = bottom.y - top.y;
				const float w = h * 0.25f;
				std::string player_name = mem.ReadString(player + 0x205);
				char buffer[255];
				sprintf_s(buffer, "Name : %s", player_name.c_str());
				ImGui::GetBackgroundDrawList()->AddText({ bottom.x - w, bottom.y }, ImColor(255, 255, 255), buffer);
			}

		}
	}
}