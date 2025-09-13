#pragma once

#include "Tout.hpp"

//TODO: Implement Linux version too
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

#define SDL_MAIN_HANDLED  
#include <SDL3/SDL.h>
#include "SDL3/SDL_main.h"
#include "SDL3/SDL_system.h"
#include "SDL3/SDL_properties.h"


struct Tarantino
{
	static bool initialise();
	static void run();
	static bool shutdown();
	static inline bool b_isRunning = false;

	struct Graphics
	{
		static int windowID;
		static int renderSurface;

		static const int APP_NUM_FRAMES_IN_FLIGHT = 2;
		static const int APP_NUM_BACK_BUFFERS = 2;
		static const int APP_SRV_HEAP_SIZE = 64;

		static void WaitForLastSubmittedFrame();
		static void CleanupRenderTarget();
		static void CreateRenderTarget();
		static float GetDeltaTime_Render();
		static float GetFPS_Render();

		// Directx 12 Data... Pray for me
		struct DX12
		{

			struct FrameContext
			{
				ID3D12CommandAllocator* CommandAllocator;
				UINT64 FenceValue;
			};

			static void CleanupDeviceD3D();
			static bool CreateDeviceD3D(HWND hWnd);

			static FrameContext* WaitForNextFrameResources();

			// Thank you imgui for impl_dx12.cpp
			struct DescriptorHeapAllocator
			{
				ID3D12DescriptorHeap* Heap = nullptr;
				D3D12_DESCRIPTOR_HEAP_TYPE  HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
				D3D12_CPU_DESCRIPTOR_HANDLE HeapStartCpu;
				D3D12_GPU_DESCRIPTOR_HANDLE HeapStartGpu;
				UINT                        HeapHandleIncrement;
				std::vector<int>            FreeIndices;

				void Create(ID3D12Device* device, ID3D12DescriptorHeap* heap)
				{
					assert(Heap == nullptr && FreeIndices.empty());
					Heap = heap;
					D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
					HeapType = desc.Type;
					HeapStartCpu = Heap->GetCPUDescriptorHandleForHeapStart();
					HeapStartGpu = Heap->GetGPUDescriptorHandleForHeapStart();
					HeapHandleIncrement = device->GetDescriptorHandleIncrementSize(HeapType);
					FreeIndices.reserve((int)desc.NumDescriptors);
					for (int n = desc.NumDescriptors; n > 0; n--)
						FreeIndices.push_back(n - 1);
				}
				void Destroy()
				{
					Heap = nullptr;
					FreeIndices.clear();
				}
				void Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
				{
					assert(FreeIndices.size() > 0);
					int idx = FreeIndices.back();
					FreeIndices.pop_back();
					out_cpu_desc_handle->ptr = HeapStartCpu.ptr + (idx * HeapHandleIncrement);
					out_gpu_desc_handle->ptr = HeapStartGpu.ptr + (idx * HeapHandleIncrement);
				}
				void Free(D3D12_CPU_DESCRIPTOR_HANDLE out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE out_gpu_desc_handle)
				{
					int cpu_idx = (int)((out_cpu_desc_handle.ptr - HeapStartCpu.ptr) / HeapHandleIncrement);
					int gpu_idx = (int)((out_gpu_desc_handle.ptr - HeapStartGpu.ptr) / HeapHandleIncrement);
					assert(cpu_idx == gpu_idx);
					FreeIndices.push_back(cpu_idx);
				}
			};

			inline static ID3D12Device*					g_pd3dDevice = nullptr;

			inline static UINT							g_frameIndex = 0;
			inline static ID3D12Fence*					g_fence;
			inline static FrameContext					g_frameContext[APP_NUM_FRAMES_IN_FLIGHT] = {};
			inline static ID3D12DescriptorHeap*			g_pd3dRtvDescHeap = nullptr;
			inline static ID3D12DescriptorHeap*			g_pd3dSrvDescHeap = nullptr;
			inline static DescriptorHeapAllocator		g_pd3dSrvDescHeapAlloc;
			inline static ID3D12CommandQueue*			g_pd3dCommandQueue = nullptr;
			inline static ID3D12GraphicsCommandList*	g_pd3dCommandList = nullptr;
			inline static HANDLE						g_fenceEvent = nullptr;
			inline static UINT64						g_fenceLastSignaledValue = 0;
			inline static IDXGISwapChain3*				g_pSwapChain = nullptr;
			inline static bool							g_SwapChainOccluded = false;
			inline static HANDLE						g_hSwapChainWaitableObject = nullptr;
			inline static ID3D12Resource*				g_mainRenderTargetResource[APP_NUM_BACK_BUFFERS] = {};
			inline static D3D12_CPU_DESCRIPTOR_HANDLE	g_mainRenderTargetDescriptor[APP_NUM_BACK_BUFFERS] = {};
		};
	};


	// SDL goodness
	struct Window
	{
		static HWND GetSDLMainWindowHandle();
		static bool SDL_Window_isValid();

		inline static SDL_Window* SDL_MainWindow;
	};



	struct Surface
	{

	};
};

