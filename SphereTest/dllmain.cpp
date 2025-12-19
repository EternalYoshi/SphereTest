// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <d3d9.h>
#include "gui\imgui\imgui.h"
#include "gui\imgui\imgui_impl_win32.h"
#include "gui\imgui\imgui_impl_dx9.h"
#include "mutex"
#include "utils/MemoryMgr.h"
#include "utils/Trampoline.h"
#include "utils\Patterns.h"
#include "code\umvc3utils.h"
#include "kiero\kiero.h"
#include <Windows.h>
#include "minhook\include\MinHook.h"
#include "code/umvc3menu.h"
#include "TheRenderer.h"
#include <atomic>
#include <mutex>
#include <unordered_set>
#include "Globals.h"

typedef long(__stdcall* EndScene)(LPDIRECT3DDEVICE9);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

//Defines the debug symbol.
#define DEBUG  

using namespace Memory::VP;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static thread_local bool threadInitialized = false;
static std::atomic<bool> hasRenderedThisFrame{ false };
static std::mutex renderMutex;
static std::unordered_set<DWORD> renderThreads;
static std::atomic<int> FrameCounter{ 0 };
static thread_local int LastRenderedFrame = -1;

// Add these globals with your other function pointers
typedef HRESULT(__stdcall* Reset)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
Reset oReset = nullptr;

static DWORD MainRenderThread = 0;
DWORD CurrentThreadId;
int CurrentFrame = 0;
EndScene oEndScene = NULL;
WNDPROC oWndProc;
static HWND window = NULL;
bool initialized = false;
bool DisplaySpheres = false;
bool ImGuiInitialized = false;

static int64 timer = GetTickCount64();


bool CreateHurtboxRenderTarget(LPDIRECT3DDEVICE9 pDevice)
{
	if (g_renderTargetCreated1) return true;

	HRESULT hr = pDevice->CreateTexture(
		1600, 900, 1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&g_pRenderTexture1,
		NULL
	);

	if (FAILED(hr)) return false;

	g_pRenderTexture1->GetSurfaceLevel(0, &g_pRenderSurface1);

	hr = pDevice->CreateDepthStencilSurface(
		1600, 900,
		D3DFMT_D24S8,
		D3DMULTISAMPLE_NONE, 0,
		TRUE,
		&g_pDepthStencilSurface1,
		NULL
	);

	if (FAILED(hr)) return false;

	g_renderTargetCreated1 = true;
	return true;
}

bool CreateHitboxRenderTarget(LPDIRECT3DDEVICE9 pDevice)
{
	if (g_renderTargetCreated2) return true;

	HRESULT hr = pDevice->CreateTexture(
		1600, 900, 1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&g_pRenderTexture2,
		NULL
	);

	if (FAILED(hr)) return false;

	g_pRenderTexture2->GetSurfaceLevel(0, &g_pRenderSurface2);

	hr = pDevice->CreateDepthStencilSurface(
		1600, 900,
		D3DFMT_D24S8,
		D3DMULTISAMPLE_NONE, 0,
		TRUE,
		&g_pDepthStencilSurface2,
		NULL
	);

	if (FAILED(hr)) return false;

	g_renderTargetCreated2 = true;
	return true;
}

void InitImGui(LPDIRECT3DDEVICE9 pDevice)
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(pDevice);

	ImGuiStyle* style = &ImGui::GetStyle();
	style->WindowRounding = 6.0f;
	style->ItemSpacing = ImVec2(7, 5.5);
	style->FrameRounding = 2.0f;
	style->FramePadding = ImVec2(6, 4.25);

}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

#ifdef DEBUG

	//switch (uMsg)
	//{
	//case WM_KILLFOCUS:
	//	TheMenu->m_bIsFocused = false;
	//	break;
	//case WM_SETFOCUS:
	//	TheMenu->m_bIsFocused = true;
	//	break;
	//	break;
	//default:
	//	break;
	//}

	//if (TheMenu->GetActiveState())
	//{
	//	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	//	return true;
	//}
#endif
	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

//Checks for valid version of the game.
static bool CheckGame()
{
	char* gameName = (char*)(_addr(0x140B12D10));

	if (strcmp(gameName, "umvc3") == 0)
	{
		return true;
	}
	else
	{
		MessageBoxA(0, "Unfortunaetly this version of the game is not compatible.\nYou need the most up to date Steam version of\nUltimate Marvel vs Capcom 3(Or the April 2017 version).", 0, MB_ICONINFORMATION);
		return false;
	}
}

long __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
	//Attempts to Track rendering threads.
	CurrentThreadId = GetCurrentThreadId();

	{
		std::lock_guard<std::mutex> lock(renderMutex);
		if (renderThreads.find(CurrentThreadId) == renderThreads.end()) {
			renderThreads.insert(CurrentThreadId);
			char buf[256];
			sprintf_s(buf, "Thread detected: %lu (Total threads: %zu)\n",
				CurrentThreadId, renderThreads.size());
			//printf(buf, "Thread detected: %lu (Total threads: %zu)\n",
			//	CurrentThreadId, renderThreads.size());
			OutputDebugStringA(buf);

			// First thread becomes the main render thread
			if (MainRenderThread == 0) {
				MainRenderThread = CurrentThreadId;
			}
		}
	}


	//For now, only do the ImGui stuff on the Main Thread.
	if (CurrentThreadId == MainRenderThread && !initialized)
	{
		D3DDEVICE_CREATION_PARAMETERS params;
		pDevice->GetCreationParameters(&params);
		window = params.hFocusWindow;
		oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
#ifdef DEBUG
		//InitImGui(pDevice);
#endif
		initialized = true;
		//hitboxRenderer = new HitboxRender();
	}



	//Becuase of the above, also ONLY render ImGui stuff on main thread.
	if (CurrentThreadId == MainRenderThread && initialized)
	{
#ifdef DEBUG
		//ImGui_ImplDX9_NewFrame();
		//ImGui_ImplWin32_NewFrame();
		//ImGui::NewFrame();
		//ImGui::GetIO().MouseDrawCursor = false;

		//if (TheMenu->GetActiveState())
		//{
		//	TheMenu->Draw();
		//}

		//ImGui::EndFrame();

		//ImGui::Render();
#endif
	}

	//This updates hitbox and hurtbox data only on the main thread once per game frame.
	if (CurrentThreadId == MainRenderThread && BeginHitboxDisplay)
	{
		UpdateSphereData(P1C1Hurtboxes, P1C2Hurtboxes, P1C3Hurtboxes,
			P2C1Hurtboxes, P2C2Hurtboxes, P2C3Hurtboxes,
			P1ShotHitSpheres, P1ShotHitCapsules, P2ShotHitSpheres, P2ShotHitCapsules);
	}

	//Checks the game mode and if valid does the imgui stuff.
	if (CheckTheMode() == true)
	{
		GetMainPointers();
		CheckIfInMatch();
		if (InMatch && sCharacter)
		{

			Emptytied = false;
			TickUpdates();
			GetPlayerData();
			GetCharacterIDs();
			GetHurtboxData();
			GetHitboxDataPart1();
			GetCameraStuff();
			GetEvenMorePlayerData();
			//EmptyTheChildLists();

			if(Player1CharNodeTree && Player2CharNodeTree)
			{
				GetChildCharacters();
				EmptyShotLists();
				GetShots();
				GetChildData();
			}
		}
		else if(!Emptytied)
		{
			RemoveAllSpheres();
			EmptyTheChildLists();
			EmptyShotLists();
		}
	}

	//// Render once per frame using thread_local tracking.
	//CurrentFrame = FrameCounter.load();

	StartRendering(pDevice);

	if (BeginHitboxDisplay)
	{
		
		if (!g_renderTargetCreated1)
		{
			CreateHurtboxRenderTarget(pDevice);
		}
		if (!g_renderTargetCreated2)
		{
			CreateHitboxRenderTarget(pDevice);
		}

		//Intended to Prevent Ghosting.
		pDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

		if (g_renderTargetCreated1 && g_renderTargetCreated2)
		{
		
			//Need to save our current render targets first.
			LPDIRECT3DSURFACE9 pOldRenderTarget1 = NULL;
			LPDIRECT3DSURFACE9 pOldDepthStencil1 = NULL;
			pDevice->GetRenderTarget(0, &pOldRenderTarget1);
			pDevice->GetDepthStencilSurface(&pOldDepthStencil1);

			LPDIRECT3DSURFACE9 pOldRenderTarget2 = NULL;
			LPDIRECT3DSURFACE9 pOldDepthStencil2 = NULL;
			pDevice->GetRenderTarget(0, &pOldRenderTarget2);
			pDevice->GetDepthStencilSurface(&pOldDepthStencil2);

			//Our textures shall be our render targets.
			pDevice->SetRenderTarget(0, g_pRenderSurface1);
			pDevice->SetDepthStencilSurface(g_pDepthStencilSurface1);

			pDevice->SetRenderTarget(0, g_pRenderSurface2);
			pDevice->SetDepthStencilSurface(g_pDepthStencilSurface2);

			//Clears textures with transparent background.
			pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
				D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
		
			RenderSpheresFromBuffer(pDevice);

			//Restores the original render targets.
			pDevice->SetRenderTarget(0, pOldRenderTarget1);
			pDevice->SetDepthStencilSurface(pOldDepthStencil1);

			if (pOldRenderTarget1) pOldRenderTarget1->Release();
			if (pOldDepthStencil1) pOldDepthStencil1->Release();

			pDevice->SetRenderTarget(0, pOldRenderTarget2);
			pDevice->SetDepthStencilSurface(pOldDepthStencil2);

			if (pOldRenderTarget2) pOldRenderTarget2->Release();
			if (pOldDepthStencil2) pOldDepthStencil2->Release();

			DrawHurtboxTexture(pDevice, g_hitboxAlpha);
			DrawHitboxTexture(pDevice, g_hitboxAlpha);
		}




		//if (BeginHitboxDisplay)
		//{
		//	RenderTheSpheres(pDevice, P1C1Hurtboxes, P1C2Hurtboxes, P1C3Hurtboxes, P2C1Hurtboxes, P2C2Hurtboxes, P2C3Hurtboxes);
		//}
		//StopRendering(pDevice);

		//LastRenderedFrame = CurrentFrame;

		//// Debug output to confirm rendering
		//char buf[256];
		//sprintf_s(buf, "Rendered spheres on thread %lu, frame %d\n", CurrentThreadId, CurrentFrame);
		//printf(buf, "Rendered spheres on thread %lu, frame %d\n", CurrentThreadId, CurrentFrame);
		//OutputDebugStringA(buf);
		

	}
	StopRendering(pDevice);

#ifdef DEBUG

	//if (CurrentThreadId == MainRenderThread && initialized)
	//{
	//	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	//}
#endif

	HRESULT result = oEndScene(pDevice);
	FrameCounter.fetch_add(1);

	return result;
}

//Backup of End Scene.
/*
long __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
	//Attempts to Track rendering threads.
	CurrentThreadId = GetCurrentThreadId();

	{
		std::lock_guard<std::mutex> lock(renderMutex);
		if (renderThreads.find(CurrentThreadId) == renderThreads.end()) {
			renderThreads.insert(CurrentThreadId);
			char buf[256];
			sprintf_s(buf, "Thread detected: %lu (Total threads: %zu)\n",
				CurrentThreadId, renderThreads.size());
			printf(buf, "Thread detected: %lu (Total threads: %zu)\n",
				CurrentThreadId, renderThreads.size());
			OutputDebugStringA(buf);

			// First thread becomes the main render thread
			if (MainRenderThread == 0) {
				MainRenderThread = CurrentThreadId;
			}
		}
	}


	//For now, only do the ImGui stuff on the Main Thread.
	if (CurrentThreadId == MainRenderThread && !initialized)
	{
		D3DDEVICE_CREATION_PARAMETERS params;
		pDevice->GetCreationParameters(&params);
		window = params.hFocusWindow;
		oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
		InitImGui(pDevice);
		initialized = true;
		//hitboxRenderer = new HitboxRender();
	}



	//Becuase of the above, also ONLY render ImGui stuff on main thread.
	if (CurrentThreadId == MainRenderThread && initialized)
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::GetIO().MouseDrawCursor = false;

		if (TheMenu->GetActiveState())
		{
			TheMenu->Draw();
		}

		ImGui::EndFrame();

		ImGui::Render();
	}

	// UPDATE sphere data only on the main thread (once per frame)
	if (CurrentThreadId == MainRenderThread && BeginHitboxDisplay)
	{
		UpdateSphereData(P1C1Hurtboxes, P1C2Hurtboxes, P1C3Hurtboxes,
			P2C1Hurtboxes, P2C2Hurtboxes, P2C3Hurtboxes);
	}


	if (CheckTheMode() == true)
	{
		GetMainPointers();
		CheckIfInMatch();
		if (InMatch)
		{
			TickUpdates();
			GetPlayerData();
			GetCharacterIDs();
			GetHurtboxData();
			GetHitboxDataPart1();
			GetCameraStuff();
			GetEvenMorePlayerData();
			GetChildCharacters();
		}
	}

	//// Render once per frame using thread_local tracking.
	//CurrentFrame = FrameCounter.load();

	StartRendering(pDevice);

	if (BeginHitboxDisplay)
	{


		//Intended to Prevent Ghosting.
		pDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		RenderSpheresFromBuffer(pDevice);


		//if (BeginHitboxDisplay)
		//{
		//	RenderTheSpheres(pDevice, P1C1Hurtboxes, P1C2Hurtboxes, P1C3Hurtboxes, P2C1Hurtboxes, P2C2Hurtboxes, P2C3Hurtboxes);
		//}
		//StopRendering(pDevice);

		//LastRenderedFrame = CurrentFrame;

		//// Debug output to confirm rendering
		//char buf[256];
		//sprintf_s(buf, "Rendered spheres on thread %lu, frame %d\n", CurrentThreadId, CurrentFrame);
		//printf(buf, "Rendered spheres on thread %lu, frame %d\n", CurrentThreadId, CurrentFrame);
		//OutputDebugStringA(buf);


	}
	StopRendering(pDevice);

	if (CurrentThreadId == MainRenderThread && initialized)
	{
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	HRESULT result = oEndScene(pDevice);
	FrameCounter.fetch_add(1);

	return result;
}

*/

void OnInitializeHook()
{
#ifdef DEBUG

	//TheMenu->Initialize();

	//Trampoline* tramp = Trampoline::MakeTrampoline(GetModuleHandle(nullptr));
	//TheMenu->tramp = tramp;

	//TheMenu->PostInit();
#endif

}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	//Waits a hot moment to ensure the game's own DX9 backend can initiate before we do anything.
	Sleep(5000);
	//Notifications->Init();
	bool attached = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D9) == kiero::Status::Success)
		{

			//AllocConsole();
			//freopen("conin$", "r", stdin);
			//freopen("conout$", "w", stdout);
			//freopen("conout$", "w", stderr);

			kiero::bind(42, (void**)&oEndScene, hkEndScene);
			attached = true;
		}
	} while (!attached);
	return true;
}

void WINAPI HookUpdate()
{
	while (true)
	{
#ifdef DEBUG
		//TheMenu->Process();
#endif

		//if (GetTickCount64() - timer <= 50) 
		//{
		//	Sleep(1);
		//	return;
		//}
		//timer = GetTickCount64();
		if (CheckTheMode() == true)
		{
			if (InMatch && sCharacter) {
				if (GetAsyncKeyState(VK_NEXT))
				{
					if (GetTickCount64() - timer <= 150) 
					{ 
					
					}
					else
					{
						timer = GetTickCount64();
						if (BeginHitboxDisplay)
						{
							BeginHitboxDisplay = false;
						}
						else
						{
							BeginHitboxDisplay = true;
						}
					}

				}
			}
			else
			{
				BeginHitboxDisplay = false;
			}
		}

		Sleep(1);
	}
}

BOOL APIENTRY DllMain(HMODULE hMod, DWORD  dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		//Checks for valid version. Does not go further if game is the wrong version.
		if (CheckGame())
		{
			//SettingsMgr->Init();
			DisableThreadLibraryCalls(hMod);
			CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
			CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HookUpdate, hMod, 0, nullptr);
			OnInitializeHook();

		}
		break;

	case DLL_PROCESS_DETACH:
		if (ImGuiInitialized)
		{
			ImGui_ImplDX9_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
		}
		kiero::shutdown();
		break;
	}
	return TRUE;
}

