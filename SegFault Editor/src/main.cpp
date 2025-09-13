#include "Tout.hpp"
#include "PacketNinja.hpp"
#include "FileWizard.hpp"
#include "DaftPunk.hpp"



#define SDL_MAIN_HANDLED     
#include <SDL3/SDL.h>
#include "SDL3/SDL_main.h"
#include "SDL3/SDL_system.h"
#include "SDL3/SDL_properties.h"



// TODO: Implement Linux version too
#ifdef _WIN32
#include <Windows.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;
#endif

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif
#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")
#include <d3d12.h>



#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_dx12.h"

#include "Rendering.h"



struct ImGui_Settings
{
	ImGui_ImplDX12_InitInfo Init_Info;
	ImGuiIO* io;
	ImGuiStyle* style;
};

bool setup_imgui(ImGui_Settings*);



int main()
{
	TOUT_LOG::Init_Log(); // Make SURE this always is launched first
	HMODULE module = LoadLibrary(L"WinPixEventRuntime.dll");

	// Linker Sanity check
	int linkResult = LINK_TEST_Logger() + LINK_TEST_FileWizard() + LINK_TEST_PacketNinja();
	if (linkResult != (43 * 3))
		CRITICAL("One of the holy trinity modules wasn't linked properly... HOW????");

	// This is where hell begins
	// First we Sort out the audio engine 
	DaftPunk::Engine::Init();
	
	// THEN we do SDL goodness
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD) == false)
	{
		OhSHITc("Error: SDL_Init(): \n{}\n", SDL_GetError());
		return -1;
	}

	Tarantino::Window::SDL_MainWindow = SDL_CreateWindow("Kill me", 1600, 900, SDL_WINDOW_RESIZABLE);
	if (!Tarantino::Window::SDL_Window_isValid())
		return -1;

	HWND hwnd = Tarantino::Window::GetSDLMainWindowHandle();


	if (Tarantino::Graphics::DX12::CreateDeviceD3D(hwnd) == false)
	{
		Tarantino::Graphics::DX12::CleanupDeviceD3D();
		SDL_DestroyWindow(Tarantino::Window::SDL_MainWindow);
		return 1;
	}

	ImGui_Settings local_imgui_settings;

	setup_imgui(&local_imgui_settings);


	ImGui_ImplSDL3_InitForD3D(Tarantino::Window::SDL_MainWindow);
	ImGui_ImplDX12_Init(&local_imgui_settings.Init_Info);

	static bool enable_vsync = false;
	
	Tarantino::b_isRunning = true;
	while (Tarantino::b_isRunning)
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			ImGui_ImplSDL3_ProcessEvent(&e);
			if (e.type == SDL_EVENT_QUIT)
				Tarantino::b_isRunning = false;
			// handle resize: recreate swapchain + RTVs + any wrapped NVRHI textures
		}

		// Handle window screen locked
		if ((Tarantino::Graphics::DX12::g_SwapChainOccluded && Tarantino::Graphics::DX12::g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) || ::IsIconic(hwnd))
		{
			::Sleep(10);
			continue;
		}
		Tarantino::Graphics::DX12::g_SwapChainOccluded = false;

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();


		ImGui::Begin("Performance");

		ImGui::Text("Deltatime: %f", Tarantino::Graphics::GetDeltaTime_Render());
		ImGui::Text("fps: %f", Tarantino::Graphics::GetFPS_Render());
		ImGui::Text(enable_vsync ? "V-sync on" : "V-sync off");
		if (ImGui::Button("Toggle V-Sync"))
			enable_vsync = !enable_vsync;

		ImGui::Text("Test .Wav file found: %d", std::filesystem::is_regular_file(std::filesystem::path("found_lost_audio.wav")));

		if (ImGui::Button("Test Audio"))
		{
			DaftPunk::PlaySoundFromFile(std::filesystem::path("found_lost_audio_22kHz_24bitPcm_trim.wav"));
			
		}
		
		
		ImGui::End();

		ImGui::ShowDemoWindow();

		ImGui::Render();

		Tarantino::Graphics::DX12::FrameContext* frameCtx = Tarantino::Graphics::DX12::WaitForNextFrameResources();
		UINT backBufferIdx = Tarantino::Graphics::DX12::g_pSwapChain->GetCurrentBackBufferIndex();
		frameCtx->CommandAllocator->Reset();

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = Tarantino::Graphics::DX12::g_mainRenderTargetResource[backBufferIdx];
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		Tarantino::Graphics::DX12::g_pd3dCommandList->Reset(frameCtx->CommandAllocator, nullptr);
		Tarantino::Graphics::DX12::g_pd3dCommandList->ResourceBarrier(1, &barrier);

		// Render Dear ImGui graphics
		const float clear_color_with_alpha[4] = {  };
		Tarantino::Graphics::DX12::g_pd3dCommandList->ClearRenderTargetView(Tarantino::Graphics::DX12::g_mainRenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, nullptr);
		Tarantino::Graphics::DX12::g_pd3dCommandList->OMSetRenderTargets(1, &Tarantino::Graphics::DX12::g_mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
		Tarantino::Graphics::DX12::g_pd3dCommandList->SetDescriptorHeaps(1, &Tarantino::Graphics::DX12::g_pd3dSrvDescHeap);

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), Tarantino::Graphics::DX12::g_pd3dCommandList);
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		Tarantino::Graphics::DX12::g_pd3dCommandList->ResourceBarrier(1, &barrier);
		Tarantino::Graphics::DX12::g_pd3dCommandList->Close();

		Tarantino::Graphics::DX12::g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&Tarantino::Graphics::DX12::g_pd3dCommandList);

		// Present
		//HRESULT hr = GraphicsManager::Graphics::DX12::g_pSwapChain->Present(1, 0);   // Yay vsync
		HRESULT hr = Tarantino::Graphics::DX12::g_pSwapChain->Present(enable_vsync, 0);   // nay vsync
		Tarantino::Graphics::DX12::g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);

		UINT64 fenceValue = Tarantino::Graphics::DX12::g_fenceLastSignaledValue + 1;
		Tarantino::Graphics::DX12::g_pd3dCommandQueue->Signal(Tarantino::Graphics::DX12::g_fence, fenceValue);
		Tarantino::Graphics::DX12::g_fenceLastSignaledValue = fenceValue;
		frameCtx->FenceValue = fenceValue;
	}
	Tarantino::Graphics::WaitForLastSubmittedFrame();

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	Tarantino::Graphics::DX12::CleanupDeviceD3D();
	SDL_DestroyWindow(Tarantino::Window::SDL_MainWindow);
	SDL_Quit();

	return 0;
}



bool setup_imgui(ImGui_Settings* p_imguiSettings)
{
	try
	{

		p_imguiSettings->Init_Info = {};
		p_imguiSettings->Init_Info.Device = Tarantino::Graphics::DX12::g_pd3dDevice;
		p_imguiSettings->Init_Info.CommandQueue = Tarantino::Graphics::DX12::g_pd3dCommandQueue;
		p_imguiSettings->Init_Info.NumFramesInFlight = Tarantino::Graphics::APP_NUM_FRAMES_IN_FLIGHT;
		p_imguiSettings->Init_Info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		p_imguiSettings->Init_Info.DSVFormat = DXGI_FORMAT_UNKNOWN;

		// Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
		p_imguiSettings->Init_Info.SrvDescriptorHeap = Tarantino::Graphics::DX12::g_pd3dSrvDescHeap;
		p_imguiSettings->Init_Info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle)
			{ return Tarantino::Graphics::DX12::g_pd3dSrvDescHeapAlloc.Alloc(out_cpu_handle, out_gpu_handle); };

		p_imguiSettings->Init_Info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
			{ return Tarantino::Graphics::DX12::g_pd3dSrvDescHeapAlloc.Free(cpu_handle, gpu_handle); };

		// THEN we do ImGui goodness
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		p_imguiSettings->io = &ImGui::GetIO();
		p_imguiSettings->io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		p_imguiSettings->io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		ImGui::StyleColorsDark();


		// Setup scaling
		p_imguiSettings->style = &ImGui::GetStyle();
		p_imguiSettings->style->ScaleAllSizes(SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay()));        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
		p_imguiSettings->style->FontScaleDpi = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)
		return true;
	}
	catch (const std::exception& e)
	{
		OhSHITc("An error happened when running setup_imgui: {} ", e.what());
		return false;
	}
}

