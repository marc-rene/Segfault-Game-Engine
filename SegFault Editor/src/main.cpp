#include "Tout.hpp"
#include "PacketNinja.hpp"
#include "FileWizard.hpp"
#include "GlobalState.h"

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
#include <nvrhi/d3d12.h>
#include <nvrhi/utils.h>

#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_dx12.h"


struct GlobalState
{
	inline static bool bRunning;
	inline static void init()
	{
		bRunning = false;

	}
};
struct ImGuiDx12
{
	ComPtr<ID3D12DescriptorHeap>  srvHeap;   // for ImGui font/texture SRVs
	DXGI_FORMAT                   rtvFormat;
	int                           framesInFlight;
};

inline static SDL_Window* SDL_MainWindow;
inline static ID3D12Device* DX12_Device;
inline static	nvrhi::IDevice* NVRHI_device;
inline static	nvrhi::ICommandList* NVRHI_cmdList;
const	nvrhi::GraphicsAPI NVRHI_api = nvrhi::GraphicsAPI::D3D12;


ComPtr<IDXGIFactory6>          factory;
ComPtr<ID3D12CommandQueue>     queue;
ComPtr<IDXGISwapChain4>        swapchain;
DXGI_FORMAT                    backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
uint32_t                       frameCount = 3;



static GlobalState globalState;
bool CreateDx12ForWindow(HWND, uint32_t, uint32_t);
bool CreateNVRHIFromDX12();
HWND GetSDLWindowHandle();
bool InitImGui(ImGuiDx12& out);
void Frame();





int main()
{
	TOUT_LOG::Init_Log(); // Make SURE this always is launched first
	globalState.init(); // sets bRunning to false

	// Linker Sanity check
	int linkResult = LINK_TEST_Logger() + LINK_TEST_FileWizard() + LINK_TEST_PacketNinja();
	if (linkResult != (43 * 3))
		CRITICAL("One of the holy trinity modules wasn't linked properly... HOW????");

	// This is where hell begins


	SDL_MainWindow = SDL_CreateWindow("Kill me",
		1600, 900, SDL_WINDOW_RESIZABLE);

	HWND hwnd = GetSDLWindowHandle();

	CreateDx12ForWindow(hwnd, 1280, 720);

	bool nvrhi_success = CreateNVRHIFromDX12();

	ImGuiDx12 imgui_dx;
	InitImGui(imgui_dx);

	globalState.bRunning = true;
	while (globalState.bRunning)
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			ImGui_ImplSDL3_ProcessEvent(&e);
			if (e.type == SDL_EVENT_QUIT) 
				globalState.bRunning = false;
			// handle resize: recreate swapchain + RTVs + any wrapped NVRHI textures
		}

		Frame();
	}

	// shutdown (reverse order)
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyWindow(SDL_MainWindow);
	SDL_Quit();

	return 0;
}



HWND GetSDLWindowHandle()
{
	HWND window_handle = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(SDL_MainWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
	if (window_handle == nullptr)
		WARNc("SDL_GetPointerProperty is returning NullPtr... just fyi");

	return window_handle;
}



bool CreateDx12ForWindow(HWND hwnd, uint32_t width, uint32_t height)
{
	UINT flags = 0;

	// [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
	ID3D12Debug* pdx12Debug = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
		pdx12Debug->EnableDebugLayer();

	flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	if (FAILED(CreateDXGIFactory2(flags, IID_PPV_ARGS(&factory))))
		return false;

	// Choose adapter (left as default; pick high-perf if you want)
	ComPtr<IDXGIAdapter1> adapter;
	for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		DXGI_ADAPTER_DESC1 desc{};
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_1;
		adapter->GetDesc1(&desc);
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr)))
			break;
	}

	if (FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&DX12_Device))))
	{
		OhSHITc("Failure to make a DX12 Device!");
		return false;
	}

	D3D12_COMMAND_QUEUE_DESC qd{};
	qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	if (FAILED(DX12_Device->CreateCommandQueue(&qd, IID_PPV_ARGS(&queue))))
		return false;

	// Setup swap chain (copied from imgui impl directx 12... thanks)
	DXGI_SWAP_CHAIN_DESC1 sd;
	{
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 2;
		sd.Width = 0;
		sd.Height = 0;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		sd.Scaling = DXGI_SCALING_STRETCH;
		sd.Stereo = FALSE;
	}

	ComPtr<IDXGISwapChain1> sc1;
	if (FAILED(factory->CreateSwapChainForHwnd(
		queue.Get(), hwnd, &sd, nullptr, nullptr, &sc1)))
		return false;

	sc1.As(&swapchain);
	factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
	return true;
}

bool CreateNVRHIFromDX12()
{
	try
	{
		nvrhi::d3d12::DeviceDesc desc = {};
		desc.pDevice = DX12_Device;
		desc.pGraphicsCommandQueue = queue.Get();

		NVRHI_device = nvrhi::d3d12::createDevice(desc).Get();
		NVRHI_cmdList = NVRHI_device->createCommandList();

		return true;
	}
	catch (const std::exception &e)
	{
		OhSHITc("ISSUE WITH NVRHI : {}", e.what());
		return false;
	}
}

bool InitImGui(ImGuiDx12& out)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	// Platform backend (SDL3)
	if (!ImGui_ImplSDL3_InitForOther(SDL_MainWindow)) // or InitForSDLRenderer; "Other" is fine for custom renderers
		return false;

	// Create a CPU-visible SRV heap for ImGui (shader-visible).
	D3D12_DESCRIPTOR_HEAP_DESC hd{};
	hd.NumDescriptors = 1; // ImGui will grow it if needed with your code changes
	hd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	hd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	if (FAILED(DX12_Device->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&out.srvHeap))))
		return false;

	out.rtvFormat = backBufferFormat;
	out.framesInFlight = (int)frameCount;

	// Renderer backend (DX12). Needs device, frames count, RT format and the SRV heap start.
	ImGui_ImplDX12_Init(
		DX12_Device,
		out.framesInFlight,
		out.rtvFormat,
		out.srvHeap.Get(),
		out.srvHeap->GetCPUDescriptorHandleForHeapStart(),
		out.srvHeap->GetGPUDescriptorHandleForHeapStart());

	return true;
}

void Frame()
{
	// Acquire backbuffer index and the native resource
	UINT bbIndex = swapchain->GetCurrentBackBufferIndex();
	ComPtr<ID3D12Resource> bb;
	swapchain->GetBuffer(bbIndex, IID_PPV_ARGS(&bb));

	// Wrap backbuffer into NVRHI texture (one-time per buffer is better; caching omitted here)
	auto textureDesc = nvrhi::TextureDesc()
		.setDimension(nvrhi::TextureDimension::Texture2D)
		.setFormat(nvrhi::Format::RGBA8_UNORM)
		.setWidth(1280)
		.setHeight(720)
		.setIsRenderTarget(true)
		.setDebugName("Swap Chain Image");

	// In this line, <type> depends on the GAPI and should be one of: D3D11_Resource, D3D12_Resource, VK_Image.
	nvrhi::TextureHandle swapChainTexture = NVRHI_device->createHandleForNativeTexture(nvrhi::ObjectTypes::D3D12_Resource, bb.Get(), textureDesc);

	// Build framebuffer
	auto framebufferDesc = nvrhi::FramebufferDesc()
		.addColorAttachment(swapChainTexture); // you can specify a particular subresource if necessary

	nvrhi::FramebufferHandle framebuffer = NVRHI_device->createFramebuffer(framebufferDesc);


	// Record & submit your scene rendering
	auto cmd = NVRHI_cmdList;
	cmd->open();
	nvrhi::GraphicsState state;
	state.framebuffer = framebuffer;
	// .. set pipelines, viewports, draw calls, etc.

	cmd->close();
	NVRHI_device->executeCommandList(cmd);
	

	// Start ImGui
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	// --- Build UI ---
	ImGui::Begin("Hello, NVRHI + SDL3 + DX12");
	ImGui::Text("It works!");
	ImGui::End();

	ImGui::Render();

	// Render ImGui via DX12 backend. It needs a command list that is recording.
	// For simplicity, create/record a native list here; in production, integrate with your frame graph.
	ComPtr<ID3D12CommandAllocator> alloc;
	ComPtr<ID3D12GraphicsCommandList> list;
	DX12_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&alloc));
	DX12_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc.Get(), nullptr, IID_PPV_ARGS(&list));

	ID3D12DescriptorHeap* heaps[] = { /* ImGui SRV heap */ ImGui::GetIO().BackendRendererUserData ? nullptr : nullptr };
	// Just set the heap explicitly:
	// heaps[0] = your ImGui SRV heap pointer; cache it when initializing
	// (Store it in a global/singleton or pass through; omitted for brevity.)

	// Transition backbuffer for render target (if not already)
	// In a real app, do proper resource state tracking;
	// NVRHI can manage states automatically for *its* work,
	// but here we issue one barrier for ImGui’s native draw:
	D3D12_RESOURCE_BARRIER rb{};
	rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	rb.Transition.pResource = bb.Get();
	rb.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	rb.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	list->ResourceBarrier(1, &rb);

	// Set RT and draw ImGui
	// (Create/keep RTVs for the swapchain buffers; omitted for brevity)
	// D3D12_CPU_DESCRIPTOR_HANDLE rtv = ... for bbIndex
	// list->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

	// Tell backend to draw
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), list.Get());

	// Transition for present
	std::swap(rb.Transition.StateBefore, rb.Transition.StateAfter);
	list->ResourceBarrier(1, &rb);
	
	list->Close();
	ID3D12CommandList* submit[] = { list.Get() };
	queue->ExecuteCommandLists(1, submit);

	// Present
	swapchain->Present(1, 0);
}
