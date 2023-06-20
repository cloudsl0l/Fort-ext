#include <atomic>
#include <mutex>
//#include <TlHelp32.h>
#include <d3d9.h>
#include <dwmapi.h>
#include <xmmintrin.h>
#include <array>
#include <vector>
#include <cstdlib>
#include <random>
#include <direct.h>
#include <fstream>
#include <string>
#include <sstream>
#include <Windows.h>
#include <iostream>

#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dwmapi.lib")
#include "Memory.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx9.h"
#include "ImGui/imgui_impl_win32.h"

#define CHEAT_NAME "Elim"
#define FORTNITE "FortniteClient-Win64-Shipping.exe"

IDirect3D9Ex* p_Object = NULL;
IDirect3DDevice9Ex* p_Device = NULL;
D3DPRESENT_PARAMETERS p_Params = { NULL };
HWND GameHwnd = NULL;
RECT GameRect = { NULL };
HWND MyHwnd = NULL;
MSG Message = { NULL };

std::uint32_t processId = 0;
uintptr_t baseAddress = 0;
ULONG64 modSize = 0;

int Width = GetSystemMetrics(SM_CXSCREEN);
int Height = GetSystemMetrics(SM_CYSCREEN);

inline uintptr_t TargetPawn = 0;
inline float ClosestDistance = FLT_MAX;
inline int FovSize = 200;
inline float Smooth = 6;
inline bool bAimbot = true;
inline bool bIsTargeting = false;

#define M_PI 3.14159265358979323846264338327950288419716939937510

void CreateOverlay()
{
	WNDCLASSEXA wcex = {
		sizeof(WNDCLASSEXA),
		0,
		DefWindowProcA,
		0,
		0,
		nullptr,
		LoadIcon(nullptr, IDI_APPLICATION),
		LoadCursor(nullptr, IDC_ARROW),
		nullptr,
		nullptr,
		("Magical Fortnite Adventures"),
		LoadIcon(nullptr, IDI_APPLICATION)
	};

	RECT Rect;
	GetWindowRect(GetDesktopWindow(), &Rect);

	RegisterClassExA(&wcex);

	MyHwnd = CreateWindowExA(NULL, ("Magical Fortnite Adventures"), ("On The Hub"), WS_POPUP, Rect.left, Rect.top, Rect.right, Rect.bottom, NULL, NULL, wcex.hInstance, NULL);


	SetWindowLong(MyHwnd, GWL_EXSTYLE, WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_LAYERED);
	MARGINS margin = { -1 };
	DwmExtendFrameIntoClientArea(MyHwnd, &margin);
	ShowWindow(MyHwnd, SW_SHOW);
	SetWindowPos(MyHwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetLayeredWindowAttributes(MyHwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
	UpdateWindow(MyHwnd);
}

HRESULT DirectXInit()
{
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
		exit(3);

	ZeroMemory(&p_Params, sizeof(p_Params));
	p_Params.Windowed = TRUE;
	p_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	p_Params.hDeviceWindow = MyHwnd;
	p_Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	p_Params.BackBufferFormat = D3DFMT_A8R8G8B8;
	p_Params.BackBufferWidth = Width;
	p_Params.BackBufferHeight = Height;
	p_Params.EnableAutoDepthStencil = TRUE;
	p_Params.AutoDepthStencilFormat = D3DFMT_D16;
	p_Params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if (FAILED(p_Object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, MyHwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_Params, 0, &p_Device)))
	{
		p_Object->Release();
		exit(4);
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui_ImplWin32_Init(MyHwnd);
	ImGui_ImplDX9_Init(p_Device);
	ImGui::StyleColorsClassic();
	ImGuiStyle* style = &ImGui::GetStyle();


	ImGui::StyleColorsClassic();
	style->WindowPadding = ImVec2(8, 8);
	style->WindowRounding = 5.0f;
	style->FramePadding = ImVec2(4, 2);
	style->FrameRounding = 0.0f;
	style->ItemSpacing = ImVec2(8, 4);
	style->ItemInnerSpacing = ImVec2(4, 4);
	style->IndentSpacing = 21.0f;
	style->ScrollbarSize = 14.0f;
	style->ScrollbarRounding = 0.0f;
	style->GrabMinSize = 10.0f;
	style->GrabRounding = 0.0f;
	style->TabRounding = 0.f;
	style->ChildRounding = 0.0f;
	style->WindowBorderSize = 1.f;
	style->ChildBorderSize = 1.f;
	style->PopupBorderSize = 0.f;
	style->FrameBorderSize = 0.f;
	style->TabBorderSize = 0.f;

	style->Colors[ImGuiCol_Text] = ImVec4(0.000f, 0.678f, 0.929f, 1.0f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.0f, 0.0263f, 0.0357f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.059f, 0.051f, 0.071f, 1.00f);
	style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.071f, 0.071f, 0.090f, 1.00f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.0f, 0.0263f, 0.0357f, 1.00f);
	style->Colors[ImGuiCol_Border] = ImColor(0.000f, 0.678f, 0.929f, 1.0f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0263f, 0.0357f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.102f, 0.090f, 0.122f, 1.000f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.102f, 0.090f, 0.122f, 1.000f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.102f, 0.090f, 0.122f, 1.000f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.102f, 0.090f, 0.122f, 1.000f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.102f, 0.090f, 0.122f, 1.000f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.102f, 0.090f, 0.122f, 1.000f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.263f, 0.357f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImColor(87, 119, 134, 255);
	style->Colors[ImGuiCol_SliderGrab] = ImColor(119, 134, 169, 150);
	style->Colors[ImGuiCol_SliderGrabActive] = ImColor(119, 134, 169, 150);
	style->Colors[ImGuiCol_Button] = ImColor(26, 23, 31, 255);
	style->Colors[ImGuiCol_ButtonHovered] = ImColor(0.102f, 0.090f, 0.122f, 1.000f);
	style->Colors[ImGuiCol_ButtonActive] = ImColor(0.102f, 0.090f, 0.122f, 1.000f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_Column] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

	style->WindowTitleAlign.x = 0.50f;
	style->FrameRounding = 0.0f;

	p_Object->Release();
	return S_OK;
}

void CleanuoD3D()
{
	if (p_Device != NULL)
	{
		p_Device->EndScene();
		p_Device->Release();
	}
	if (p_Object != NULL)
	{
		p_Object->Release();
	}
}

//oof my code was clean until I pasted this in below smh

#define M_PI 3.14159265358979323846264338327950288419716939937510

class Vector2
{
public:
	Vector2() : x(0.f), y(0.f)
	{

	}

	Vector2(double _x, double _y) : x(_x), y(_y)
	{

	}
	~Vector2()
	{

	}

	double x;
	double y;
};
class Vector3
{
public:
	Vector3() : x(0.f), y(0.f), z(0.f)
	{

	}

	Vector3(double _x, double _y, double _z) : x(_x), y(_y), z(_z)
	{

	}
	~Vector3()
	{

	}

	double x;
	double y;
	double z;

	inline double Dot(Vector3 v)
	{
		return x * v.x + y * v.y + z * v.z;
	}

	inline double Distance(Vector3 v)
	{
		return double(sqrtf(powf(v.x - x, 2.0) + powf(v.y - y, 2.0) + powf(v.z - z, 2.0)));
	}

	inline double Length() {
		return sqrt(x * x + y * y + z * z);
	}

	Vector3 operator+(Vector3 v)
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	Vector3 operator-(Vector3 v)
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	Vector3 operator*(double flNum) { return Vector3(x * flNum, y * flNum, z * flNum); }
};
struct FQuat
{
	double x;
	double y;
	double z;
	double w;
};
struct FTransform
{
	FQuat rot;
	Vector3 translation;
	char pad[4];
	Vector3 scale;
	char pad1[4];
	D3DMATRIX ToMatrixWithScale()
	{
		D3DMATRIX m;
		m._41 = translation.x;
		m._42 = translation.y;
		m._43 = translation.z;

		float x2 = rot.x + rot.x;
		float y2 = rot.y + rot.y;
		float z2 = rot.z + rot.z;

		float xx2 = rot.x * x2;
		float yy2 = rot.y * y2;
		float zz2 = rot.z * z2;
		m._11 = (1.0f - (yy2 + zz2)) * scale.x;
		m._22 = (1.0f - (xx2 + zz2)) * scale.y;
		m._33 = (1.0f - (xx2 + yy2)) * scale.z;

		float yz2 = rot.y * z2;
		float wx2 = rot.w * x2;
		m._32 = (yz2 - wx2) * scale.z;
		m._23 = (yz2 + wx2) * scale.y;

		float xy2 = rot.x * y2;
		float wz2 = rot.w * z2;
		m._21 = (xy2 - wz2) * scale.y;
		m._12 = (xy2 + wz2) * scale.x;

		float xz2 = rot.x * z2;
		float wy2 = rot.w * y2;
		m._31 = (xz2 + wy2) * scale.z;
		m._13 = (xz2 - wy2) * scale.x;

		m._14 = 0.0f;
		m._24 = 0.0f;
		m._34 = 0.0f;
		m._44 = 1.0f;

		return m;
	}
};

template<class type>
class tarray
{
public:
	tarray() : data(nullptr), count(std::int32_t()), maxx(std::int32_t()) { }
	tarray(type* data, std::int32_t count, std::int32_t maxx) : data(data), count(count), maxx(maxx) { }

	const bool is_valid() const noexcept
	{
		return !(this->data == nullptr);
	}

	const std::int32_t size() const noexcept
	{
		return this->count;
	}

	type& operator[](std::int32_t index) noexcept
	{
		return this->data[index];
	}

	const type& operator[](std::int32_t index) const noexcept
	{
		return this->data[index];
	}

	bool is_valid_index(std::int32_t index) const noexcept
	{
		return index < this->size();
	}

protected:
	type* data;
	std::int32_t count;
	std::int32_t maxx;
};

class Util {
public:

	static void open_binary_file(const std::string& file, std::vector<uint8_t>& data)
	{
		std::ifstream fstr(file, std::ios::binary);
		fstr.unsetf(std::ios::skipws);
		fstr.seekg(0, std::ios::end);

		const auto file_size = fstr.tellg();

		fstr.seekg(NULL, std::ios::beg);
		data.reserve(static_cast<uint32_t>(file_size));
		data.insert(data.begin(), std::istream_iterator<uint8_t>(fstr), std::istream_iterator<uint8_t>());
	}
	static int get_fps()
	{
		using namespace std::chrono;
		static int count = 0;
		static auto last = high_resolution_clock::now();
		auto now = high_resolution_clock::now();
		static int fps = 0;

		count++;

		if (duration_cast<milliseconds>(now - last).count() > 1000) {
			fps = count;
			count = 0;
			last = now;
		}

		return fps;
	}
	static void PrintPtr(std::string text, uintptr_t ptr) {
		std::cout << text << ptr << std::endl;
	}
	static void Print2D(std::string text, Vector2 pos) {
		std::cout << text << pos.x << ", " << pos.y << std::endl;
	}
	static void Print3D(std::string text, Vector3 pos) {
		std::cout << text << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
	}
	static double GetCrossDistance(double x1, double y1, double x2, double y2) {
		return sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
	}
	static D3DMATRIX MatrixMultiplication(D3DMATRIX pM1, D3DMATRIX pM2)
	{
		D3DMATRIX pOut;
		pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
		pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
		pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
		pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
		pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
		pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
		pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
		pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
		pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
		pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
		pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
		pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
		pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
		pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
		pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
		pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;

		return pOut;
	}
	static D3DMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0)) {
		float radPitch = (rot.x * float(M_PI) / 180.f);
		float radYaw = (rot.y * float(M_PI) / 180.f);
		float radRoll = (rot.z * float(M_PI) / 180.f);

		float SP = sinf(radPitch);
		float CP = cosf(radPitch);
		float SY = sinf(radYaw);
		float CY = cosf(radYaw);
		float SR = sinf(radRoll);
		float CR = cosf(radRoll);

		D3DMATRIX matrix;
		matrix.m[0][0] = CP * CY;
		matrix.m[0][1] = CP * SY;
		matrix.m[0][2] = SP;
		matrix.m[0][3] = 0.f;

		matrix.m[1][0] = SR * SP * CY - CR * SY;
		matrix.m[1][1] = SR * SP * SY + CR * CY;
		matrix.m[1][2] = -SR * CP;
		matrix.m[1][3] = 0.f;

		matrix.m[2][0] = -(CR * SP * CY + SR * SY);
		matrix.m[2][1] = CY * SR - CR * SP * SY;
		matrix.m[2][2] = CR * CP;
		matrix.m[2][3] = 0.f;

		matrix.m[3][0] = origin.x;
		matrix.m[3][1] = origin.y;
		matrix.m[3][2] = origin.z;
		matrix.m[3][3] = 1.f;

		return matrix;
	}

};

namespace LocalPtrs {
	inline uintptr_t Player = 0; //our local pawn (our player)
	inline uintptr_t Mesh = 0;
	inline uintptr_t PlayerState = 0;
	inline uintptr_t RootComponent = 0;
	inline uintptr_t LocalPlayers = 0;
	inline uintptr_t PlayerController = 0;
	inline uintptr_t Gworld = 0; //uworld != gworld u little fuckers
}

Vector3 GetBoneWithRotation(uintptr_t mesh, int id)
{
	uintptr_t bonearray = 0;
	bonearray = Read<uintptr_t>(mesh + 0x600);
	if (!bonearray) bonearray = Read<uintptr_t>(mesh + 0x600 + 0x10);

	FTransform ComponentToWorld = Read<FTransform>(mesh + 0x240);

	FTransform bone = Read<FTransform>(bonearray + (id * 0x60));

	D3DMATRIX Matrix;
	Matrix = Util::MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());

	return Vector3(Matrix._41, Matrix._42, Matrix._43);
}

struct CamewaDescwipsion
{
	Vector3 Location;
	Vector3 Rotation;
	float FieldOfView;
	char Useless[0x18];
};

inline CamewaDescwipsion vCamera;

CamewaDescwipsion GetViewPoint()
{
	char v1; // r8
	CamewaDescwipsion ViewPoint = Read<CamewaDescwipsion>(baseAddress + 0xF0E8BD0);
	BYTE* v2 = (BYTE*)&ViewPoint;
	int i; // edx
	__int64 result; // rax

	v1 = 0x40;
	for (i = 0; i < 0x40; ++i)
	{
		*v2 ^= v1;
		result = (unsigned int)(i + 0x17);
		v1 += i + 0x17;
		v2++;
	}

	return ViewPoint;
}

Vector2 ProjectWorldToScreen(Vector3 WorldLocation)
{

	vCamera = GetViewPoint(); //get ur players newest view angles

	D3DMATRIX tempMatrix = Util::Matrix(vCamera.Rotation);

	Vector3 vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
	Vector3 vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
	Vector3 vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

	Vector3 vDelta = WorldLocation - vCamera.Location;
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

	if (vTransformed.z < 1.f)
		vTransformed.z = 1.f;

	return Vector2((Width / 2.0f) + vTransformed.x * (((Width / 2.0f) / tanf(vCamera.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z, (Height / 2.0f) - vTransformed.y * (((Width / 2.0f) / tanf(vCamera.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z);
}

void DrawCornerBox(int X, int Y, int W, int H, const ImColor color, int thickness) {
	float lineW = (W / 3);
	float lineH = (H / 3);

	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X, Y + lineH), color, thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X + lineW, Y), color, thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y), ImVec2(X + W, Y), color, thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y), ImVec2(X + W, Y + lineH), color, thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H - lineH), ImVec2(X, Y + H), color, thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H), ImVec2(X + lineW, Y + H), color, thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y + H), ImVec2(X + W, Y + H), color, thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y + H - lineH), ImVec2(X + W, Y + H), color, thickness);
}

void ActorLoop() {
	//I will link dumps.host to each offset, on the website it might be outdated but it gives you a understanding of the sdk

	//get address
	{
		LocalPtrs::Gworld = Read<uint64_t>(baseAddress + 0xF1BEEE8); //https://fn.dumps.host/offsets?offset=GWorld
		if (!LocalPtrs::Gworld) return;

		uintptr_t GameInstance = Read<uint64_t>(LocalPtrs::Gworld + 0x1b8); //https://fn.dumps.host/?class=UWorld&member=OwningGameInstance
		if (!GameInstance) return;

		LocalPtrs::LocalPlayers = Read<uint64_t>(Read<uint64_t>(GameInstance + 0x38)); //https://fn.dumps.host/?class=UGameInstance&member=LocalPlayers
		if (!LocalPtrs::LocalPlayers) return;

		LocalPtrs::PlayerController = Read<uint64_t>(LocalPtrs::LocalPlayers + 0x30); //https://fn.dumps.host/?class=UPlayer&member=PlayerController
		if (!LocalPtrs::PlayerController) return;

		LocalPtrs::Player = Read<uint64_t>(LocalPtrs::PlayerController + 0x330); //https://fn.dumps.host/?class=APlayerController&member=AcknowledgedPawn
		if (!LocalPtrs::Player) return;

		LocalPtrs::RootComponent = Read<uint64_t>(LocalPtrs::Player + 0x190); //https://fn.dumps.host/?class=AActor&member=RootComponent
		if (!LocalPtrs::RootComponent) return;

	}

	//get player array then loop through it
	{
		uintptr_t GameState = Read<uintptr_t>(LocalPtrs::Gworld + 0x158); //https://fn.dumps.host/?class=UWorld&member=GameState
		if (!GameState) return;

		uintptr_t PlayerArrayOffset = 0x2a0; //https://fn.dumps.host/?class=AGameStateBase&member=PlayerArray
		//PlayerArray is stored as an array in the sdk, it consist of a list of all the player states in the current game
		uintptr_t PlayerArray = Read<uintptr_t>(GameState + PlayerArrayOffset);

		int Num = Read<int>(GameState + (PlayerArrayOffset + sizeof(uintptr_t))); //reads the total number of player states in this array

		for (int i = 0; i < Num; i++) {


			uintptr_t PlayerState = Read<uintptr_t>(PlayerArray + (i * sizeof(uintptr_t))); //the size of the pointer in the array we are reading is the size of uintptr_t
			if (!PlayerState) continue; 

			uintptr_t Player = Read<uintptr_t>(PlayerState + 0x300);
			if (!Player) continue;
			if (Player == LocalPtrs::Player) continue;

			uintptr_t Mesh = Read<uintptr_t>(Player + 0x310);
			if (!Mesh) continue;

			Vector3 Head3D = GetBoneWithRotation(Mesh, 68);
			Vector2 Head2D = ProjectWorldToScreen(Head3D); 
			//Util::Print3D("Head3D: ", Head3D);
			//Util::Print2D("Head2D: ", Head2D);

			Vector3 Bottom3D = GetBoneWithRotation(Mesh, 0);
			Vector2 Bottom2D = ProjectWorldToScreen(Bottom3D);

			float BoxHeight = (float)(Head2D.y - Bottom2D.y);
			float CornerHeight = abs(Head2D.y - Bottom2D.y);
			float CornerWidth = BoxHeight * 0.80;

			ImGui::GetOverlayDrawList()->AddLine(ImVec2((Width / 2), Height / 2), ImVec2(Head2D.x, Head2D.y), IM_COL32(0, 173, 237, 255));
		
			DrawCornerBox(Head2D.x - (CornerWidth / 2), Head2D.y, CornerWidth, CornerHeight, IM_COL32(0, 173, 237, 255), 1.5);

			auto dist = Util::GetCrossDistance(Head2D.x, Head2D.y, Width / 2, Height / 2);
			if (dist < FovSize && dist < ClosestDistance) {
				ClosestDistance = dist;
				TargetPawn = Player;
			}
		}
	}
}

void aimbot() {
	if (!TargetPawn) return;

	auto mesh = Read<uintptr_t>(TargetPawn + 0x310); //https://fn.dumps.host/?class=ACharacter&member=Mesh
	if (!mesh) {
		ClosestDistance = FLT_MAX;
		TargetPawn = NULL;
		bIsTargeting = FALSE;
	}
	Vector3 Head3D = GetBoneWithRotation(mesh, 68);
	Vector2 Head2D = ProjectWorldToScreen(Head3D);

	auto distance = Util::GetCrossDistance(Head2D.x, Head2D.y, Width / 2, Height / 2);
	if (distance > FovSize or Head2D.x == 0 or Head2D.y == 0) {
		ClosestDistance = FLT_MAX;
		TargetPawn = NULL;
		bIsTargeting = FALSE;
	}

	float x = Head2D.x; float y = Head2D.y;
	float AimSpeed = Smooth;

	Vector2 ScreenCenter = { (double)Width / 2 , (double)Height / 2 };
	Vector2 Target;

	if (x != 0)
	{
		if (x > ScreenCenter.x)
		{
			Target.x = -(ScreenCenter.x - x);
			Target.x /= AimSpeed;
			if (Target.x + ScreenCenter.x > ScreenCenter.x * 2) Target.x = 0;
		}

		if (x < ScreenCenter.x)
		{
			Target.x = x - ScreenCenter.x;
			Target.x /= AimSpeed;
			if (Target.x + ScreenCenter.x < 0) Target.x = 0;
		}
	}
	if (y != 0)
	{
		if (y > ScreenCenter.y)
		{
			Target.y = -(ScreenCenter.y - y);
			Target.y /= AimSpeed;
			if (Target.y + ScreenCenter.y > ScreenCenter.y * 2) Target.y = 0;
		}

		if (y < ScreenCenter.y)
		{
			Target.y = y - ScreenCenter.y;
			Target.y /= AimSpeed;
			if (Target.y + ScreenCenter.y < 0) Target.y = 0;
		}
	}

	mouse_event(MOUSEEVENTF_MOVE, Target.x, Target.y, NULL, NULL);

}

void render() {
	char fpsinfo[64];
	sprintf(fpsinfo, ("FPS: %03d"), Util::get_fps());
	ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), 15, ImVec2(50, 50), ImColor(0, 255, 0), fpsinfo);

	//ImGui::GetOverlayDrawList()->AddRect(ImVec2((1920 / 2) - 100, (1080 / 2) - 100), ImVec2((1920 / 2) + 100, (1080 / 2) + 100), ImColor(0, 255, 0), 0, 0, 3);
	ActorLoop();
	if (GetAsyncKeyState(VK_RBUTTON) && bAimbot) {
		bIsTargeting = true;
		aimbot();
	}
	else {
		bIsTargeting = false;
	}
}

WPARAM MainLoop()
{
	static RECT old_rc;
	ZeroMemory(&Message, sizeof(MSG));

	while (Message.message != WM_QUIT)
	{
		if (PeekMessage(&Message, MyHwnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		HWND hwnd_active = GetForegroundWindow();
		if (GetAsyncKeyState(0x23) & 1)
			exit(8);

		if (hwnd_active == GameHwnd) {
			HWND hwndtest = GetWindow(hwnd_active, GW_HWNDPREV);
			SetWindowPos(MyHwnd, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		RECT rc;
		POINT xy;

		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		GetClientRect(GameHwnd, &rc);
		ClientToScreen(GameHwnd, &xy);
		rc.left = xy.x;
		rc.top = xy.y;

		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = GameHwnd;
		io.DeltaTime = 1.0f / 60.0f;

		POINT p;
		GetCursorPos(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;

		if (GetAsyncKeyState(0x1)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else
			io.MouseDown[0] = false;
		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom)
		{

			old_rc = rc;

			Width = rc.right;
			Height = rc.bottom;

			p_Params.BackBufferWidth = Width;
			p_Params.BackBufferHeight = Height;
			SetWindowPos(MyHwnd, (HWND)0, xy.x, xy.y, Width, Height, SWP_NOREDRAW);
			p_Device->Reset(&p_Params);
		}
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//do shit
		render();
		//Menu();

		ImGui::EndFrame();
		p_Device->SetRenderState(D3DRS_ZENABLE, false);
		p_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		p_Device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
		p_Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
		if (p_Device->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			p_Device->EndScene();
		}
		HRESULT result = p_Device->Present(NULL, NULL, NULL, NULL);

		if (result == D3DERR_DEVICELOST && p_Device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			p_Device->Reset(&p_Params);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanuoD3D();
	DestroyWindow(MyHwnd);

	return Message.wParam;
}


auto main(int argc, char** argv) -> int
{
	if (argc < 2)
	{
		std::printf("[!] please provide a path to a driver...\n");
		return -1;
	}

	std::vector<std::uint8_t> driver_data;
	Util::open_binary_file(argv[1], driver_data);
	std::printf("[+] loaded driver off disk...\n");

	const auto [result, driver_base] =
		mapper::map_driver
		(
			GetCurrentProcessId(), // you can map the driver into whatever context you want...
			driver_data.data(),
			driver_data.size(),
			nullptr // you can pass your structure here...
		);

	std::printf("[+] driver mapping result -> 0x%x (0 == mapper_error::error_success)\n", result);
	std::printf("[+] driver base address -> 0x%p\n", driver_base);

	LoadLibrary(L"user32.dll");

	processId = GetProcess(L"notepad.exe");
	if (!processId)
	{
		printf("[%s] Waiting for Fortnite to be launched...\n", CHEAT_NAME);

		while (true)
		{
			if (!processId) processId = GetProcess(L"notepad.exe");
			else break;

			Sleep(2000);
		}
	}

	baseAddress = GetModuleBaseAddress(FORTNITE);
	if (!baseAddress)
	{
		printf("[%s:1] Oopsie, something went wrong. Is the Driver loaded ?\n", CHEAT_NAME);

		while (true)
		{
			if (!baseAddress) baseAddress = GetModuleBaseAddress(FORTNITE);
			else break;

			Sleep(2000);
		}
	}

	modSize = GetModuleSize(FORTNITE);

	//if (modSize == NULL)
	//	exit(0);

	printf("Process ID   : 0x%p\n", processId);
	printf("Base address : 0x%p\n", baseAddress);
	printf("Module size  : 0x%p\n", modSize);

	CreateOverlay();
	DirectXInit();
	MainLoop();

}