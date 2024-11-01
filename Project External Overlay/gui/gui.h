#include "../imgui/imgui_impl_dx11.h"
#include <d3d11.h>
#include "../gui/vector.h"

class GUI
{
public:
	GUI();
	~GUI();

	// Data
	ID3D11Device*			g_pd3dDevice = nullptr;
	ID3D11DeviceContext*	g_pd3dDeviceContext = nullptr;
	IDXGISwapChain*			g_pSwapChain = nullptr;
	bool					g_SwapChainOccluded = false;
	UINT					g_ResizeWidth = 0, g_ResizeHeight = 0;
	ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

	// Forward declarations of helper functions
	bool CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();

	void RenderMenu();			// Menu ImGui
	void Hack();				// Code Hack

	int selectedSidebar = 0;

	bool b_godmode = false,  b_armor = false, b_grenade = false, b_aimbot = false;
	
	// Aimbot
	const char* target[2] = { "Head", "Body" };
	int target_current = 0;


	// ESP
	bool b_2d_box = false, b_line = false, b_2d_health = false, b_name;

	bool WordToScreen(Vector3 position, Vector2& screen, float matrix[16], int windows_width, int windows_height)
	{
		Vector4 clipCoords;
		clipCoords.x = position.x * matrix[0] + position.y * matrix[4] + position.z * matrix[8] + matrix[12];
		clipCoords.y = position.x * matrix[1] + position.y * matrix[5] + position.z * matrix[9] + matrix[13];
		clipCoords.z = position.x * matrix[2] + position.y * matrix[6] + position.z * matrix[10] + matrix[14];
		clipCoords.w = position.x * matrix[3] + position.y * matrix[7] + position.z * matrix[11] + matrix[15];

		if (clipCoords.w < 0.1f)
		{
			return false;
		}

		Vector3 NDC;
		NDC.x = clipCoords.x / clipCoords.w;
		NDC.y = clipCoords.y / clipCoords.w;
		NDC.z = clipCoords.z / clipCoords.w;

		screen.x = (windows_width / 2 * NDC.x) + (NDC.x + windows_width / 2);
		screen.y = -(windows_height / 2 * NDC.y) + (NDC.y + windows_height / 2);

		return true;
	}

	// Bar Health
	void DrawVerticalBar(float x, float y, float width, float height, int value, int max_value, ImU32 valueColor = IM_COL32(0, 255, 0, 255))
	{
		if (value < 0) value = 0;
		if (value > max_value) value = max_value;

		// Calculate value percentage
		float valuePercentage = (float)value / (float)max_value;

		// Colors
		ImU32 borderColor = IM_COL32(255, 255, 255, 255);		// White
		ImU32 backgroundColor = IM_COL32(100, 100, 100, 255);	// Gray

		// Draw Background
		ImVec2 p_min = ImVec2(x, y);
		ImVec2 p_max = ImVec2(x + width, y + height);
		ImGui::GetBackgroundDrawList()->AddRectFilled(p_min, p_max, backgroundColor);

		// Draw value
		float healthHeight = height * valuePercentage;
		ImVec2 value_min = ImVec2(x, y + height - healthHeight);
		ImVec2 value_max = ImVec2(x + width, y + height);
		ImGui::GetBackgroundDrawList()->AddRectFilled(value_min, value_max, valueColor);

		// Draw border
		ImGui::GetBackgroundDrawList()->AddRect(p_min, p_max, borderColor);
	}

};

