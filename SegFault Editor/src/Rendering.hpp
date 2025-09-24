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
#include <d3dx12.h>
#include "DirectXMath.h"
#include "StepTimer.h"

#define SDL_MAIN_HANDLED  
#include <SDL3/SDL.h>
#include "SDL3/SDL_main.h"
#include "SDL3/SDL_system.h"
#include "SDL3/SDL_properties.h"

#include "imgui.h"
#include "backends/imgui_impl_dx12.h"

//#define LIGHTTYPE_DISABLED		0	
//#define LIGHTTYPE_STATIC		1	// Only generates lightmap
//#define LIGHTTYPE_STATIONARY	2	// Lightmap AND reactive
//#define LIGHTTYPE_MOVABLE		3	// No lightmap, just reactive




//// Assign a name to the object to aid with debugging.
//#if defined(_DEBUG) || defined(DBG)
//inline void SetName(ID3D12Object* pObject, LPCWSTR name)
//{
//	pObject->SetName(name);
//}
//inline void SetNameIndexed(ID3D12Object* pObject, LPCWSTR name, UINT index)
//{
//	WCHAR fullName[50];
//	if (swprintf_s(fullName, L"%s[%u]", name, index) > 0)
//	{
//		pObject->SetName(fullName);
//	}
//}
//#else
//inline void SetName(ID3D12Object*, LPCWSTR)
//{
//}
//inline void SetNameIndexed(ID3D12Object*, LPCWSTR, UINT)
//{
//}
//#endif
//// Naming helper for ComPtr<T>.
//// Assigns the name of the variable as the name of the object.
//// The indexed variant will include the index in the name of the object.
//#define NAME_D3D12_OBJECT(x) SetName((x).Get(), L#x)
//#define NAME_D3D12_OBJECT_INDEXED(x, n) SetNameIndexed((x)[n].Get(), L#x, n)
//inline std::string HrToString(HRESULT hr)
//{
//	char s_str[64] = {};
//	sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
//	return std::string(s_str);
//}
//class HrException : public std::runtime_error
//{
//public:
//	HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
//	HRESULT Error() const { return m_hr; }
//private:
//	const HRESULT m_hr;
//};

struct Tarantino
{
	//static bool initialise();
	//static void run();
	static bool shutdown();
	static inline std::atomic<bool> b_isRunning = false;

	struct Graphics
	{

		static int windowID;
		static int renderSurface;

		static const int APP_NUM_FRAMES_IN_FLIGHT = 2;


		static const int APP_NUM_ACTIVE_LIGHTS = 8;
		static const int APP_SRV_HEAP_SIZE = 64;

		// Command list submissions from main thread.
		static const int CommandListCount = 3;
		static const int CommandListPre = 0;
		static const int CommandListMid = 1;
		static const int CommandListPost = 2;

		static void WaitForLastSubmittedFrame();
		static void CleanupRenderTarget();
		static void CreateRenderTarget();
		static void FinishRenderFrame();
		static float GetDeltaTime_Render();
		static float GetFPS_Render();

		//class Camera
		//{
		//public:
		//	Camera();
		//	~Camera();
		//
		//	void Get3DViewProjMatrices(DirectX::XMFLOAT4X4* view, DirectX::XMFLOAT4X4* proj, float fovInDegrees, float screenWidth, float screenHeight);
		//	void Reset();
		//	void Set(DirectX::XMVECTOR p_location, DirectX::XMVECTOR p_direction_vector, DirectX::XMVECTOR p_up);
		//	static Camera* get();
		//	void RotateYaw(float deg);
		//	void RotatePitch(float deg);
		//	void GetOrthoProjMatrices(DirectX::XMFLOAT4X4* view, DirectX::XMFLOAT4X4* proj, float width, float height);
		//	DirectX::XMVECTOR location; // Where the camera is in world space. Z increases into of the screen when using LH coord system (which we are and DX uses)
		//	DirectX::XMVECTOR direction_vector; // What the camera is looking at (world origin)
		//	DirectX::XMVECTOR mUp; // Which way is up
		//private:
		//	static Camera* mCamera;
		//};

		struct Settings
		{
			inline static bool r_enable_vsync = false;
		};
		// Directx 12 Data... Pray for me
		struct DX12
		{

			struct FrameContext
			{
				ID3D12CommandAllocator* CommandAllocator;
				UINT64 FenceValue;
			};

			static void Init();
			static void OnUpdate();
			static void OnInit();

			static void StartFrame();

			// Todo: make public/private seperation cause this is about to get confusing REAL damn quick
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

			inline static ID3D12Device* g_pd3dDevice = nullptr;


			inline static UINT							g_frameIndex = 0;
			inline static ID3D12Fence* g_fence;
			inline static FrameContext					g_frameContext[APP_NUM_FRAMES_IN_FLIGHT] = {};
			inline static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = nullptr;
			inline static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
			inline static DescriptorHeapAllocator		g_pd3dSrvDescHeapAlloc;
			inline static ID3D12CommandQueue* g_pd3dCommandQueue = nullptr;
			inline static ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;
			inline static HANDLE						g_fenceEvent = nullptr;
			inline static UINT64						g_fenceLastSignaledValue = 0;
			inline static IDXGISwapChain3* g_pSwapChain = nullptr;
			inline static bool							g_SwapChainOccluded = false;
			inline static HANDLE						g_hSwapChainWaitableObject = nullptr;
			inline static ID3D12Resource* g_mainRenderTargetResource[APP_NUM_FRAMES_IN_FLIGHT] = {};
			inline static D3D12_CPU_DESCRIPTOR_HANDLE	g_mainRenderTargetDescriptor[APP_NUM_FRAMES_IN_FLIGHT] = {};
		};



		class D3D12Multithreading
		{
		public:
			D3D12Multithreading(UINT width, UINT height, std::wstring name);

			static D3D12Multithreading* Get() { return s_app; }

			void OnInit();
			void OnUpdate();
			void OnRender();
			void OnDestroy();


		private:
			struct TextureResource
			{
				UINT Width;
				UINT Height;
				UINT MipLevels;
				DXGI_FORMAT Format;
				struct DataProperties
				{
					UINT Offset;
					UINT Size;
					UINT Pitch;
				} Data[D3D12_REQ_MIP_LEVELS];
			};

			struct LightData
			{
				UINT8 type;
				DirectX::XMFLOAT4 position;
				DirectX::XMFLOAT4 direction;
				DirectX::XMFLOAT4 color;
				DirectX::XMFLOAT4 falloff;

				DirectX::XMFLOAT4X4 view;
				DirectX::XMFLOAT4X4 projection;
			};

			struct SceneConstantBuffer
			{
				DirectX::XMFLOAT4X4 model;
				DirectX::XMFLOAT4X4 view;
				DirectX::XMFLOAT4X4 projection;
				DirectX::XMFLOAT4 ambientColor;
				BOOL sampleShadowMap;
				BOOL padding[3];        // Must be aligned to be made up of N float4s.
				LightData lights[APP_NUM_ACTIVE_LIGHTS];
			};

			class FrameResource
			{
			public:
				ID3D12CommandList* m_batchSubmit[APP_NUM_FRAMES_IN_FLIGHT * 2 + CommandListCount];

				ComPtr<ID3D12CommandAllocator> m_commandAllocators[CommandListCount];
				ComPtr<ID3D12GraphicsCommandList> m_commandLists[CommandListCount];

				ComPtr<ID3D12CommandAllocator> m_shadowCommandAllocators[APP_NUM_FRAMES_IN_FLIGHT];
				ComPtr<ID3D12GraphicsCommandList> m_shadowCommandLists[APP_NUM_FRAMES_IN_FLIGHT];

				ComPtr<ID3D12CommandAllocator> m_sceneCommandAllocators[APP_NUM_FRAMES_IN_FLIGHT];
				ComPtr<ID3D12GraphicsCommandList> m_sceneCommandLists[APP_NUM_FRAMES_IN_FLIGHT];

				UINT64 m_fenceValue;

			private:
				ComPtr<ID3D12PipelineState> m_pipelineState;
				ComPtr<ID3D12PipelineState> m_pipelineStateShadowMap;
				ComPtr<ID3D12Resource> m_shadowTexture;
				D3D12_CPU_DESCRIPTOR_HANDLE m_shadowDepthView;
				ComPtr<ID3D12Resource> m_shadowConstantBuffer;
				ComPtr<ID3D12Resource> m_sceneConstantBuffer;
				SceneConstantBuffer* mp_shadowConstantBufferWO;			// WRITE-ONLY pointer to the shadow pass constant buffer.
				SceneConstantBuffer* mp_sceneConstantBufferWO;			// WRITE-ONLY pointer to the scene pass constant buffer.
				D3D12_GPU_DESCRIPTOR_HANDLE m_nullSrvHandle;			// Null SRV for out of bounds behavior.
				D3D12_GPU_DESCRIPTOR_HANDLE m_shadowDepthHandle;
				D3D12_GPU_DESCRIPTOR_HANDLE m_shadowCbvHandle;
				D3D12_GPU_DESCRIPTOR_HANDLE m_sceneCbvHandle;

			public:
				FrameResource(ID3D12Device* pDevice, ID3D12PipelineState* pPso, ID3D12PipelineState* pShadowMapPso, ID3D12DescriptorHeap* pDsvHeap, ID3D12DescriptorHeap* pCbvSrvHeap, D3D12_VIEWPORT* pViewport, UINT frameResourceIndex);
				~FrameResource();

				void Bind(ID3D12GraphicsCommandList* pCommandList, BOOL scenePass, D3D12_CPU_DESCRIPTOR_HANDLE* pRtvHandle, D3D12_CPU_DESCRIPTOR_HANDLE* pDsvHandle);
				void Init();
				void SwapBarriers();
				void Finish();
			//	void WriteConstantBuffers(D3D12_VIEWPORT* pViewport, Camera* pSceneCamera, Camera lightCams[APP_NUM_ACTIVE_LIGHTS], LightData lights[APP_NUM_ACTIVE_LIGHTS]);
			};




			// Pipeline objects.
			CD3DX12_VIEWPORT m_viewport;
			CD3DX12_RECT m_scissorRect;
			ComPtr<IDXGISwapChain3> m_swapChain;
			ComPtr<ID3D12Device> m_device;
			ComPtr<ID3D12Resource> m_renderTargets[APP_NUM_FRAMES_IN_FLIGHT];
			ComPtr<ID3D12Resource> m_depthStencil;
			ComPtr<ID3D12CommandAllocator> m_commandAllocator;
			ComPtr<ID3D12CommandQueue> m_commandQueue;
			ComPtr<ID3D12RootSignature> m_rootSignature;
			ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
			ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
			ComPtr<ID3D12DescriptorHeap> m_cbvSrvHeap;
			ComPtr<ID3D12DescriptorHeap> m_samplerHeap;
			ComPtr<ID3D12PipelineState> m_pipelineState;
			ComPtr<ID3D12PipelineState> m_pipelineStateShadowMap;

			// App resources.
			D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
			D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
			//ComPtr<ID3D12Resource> m_textures[_countof(SampleAssets::Textures)];
			//ComPtr<ID3D12Resource> m_textureUploads[_countof(SampleAssets::Textures)];
			ComPtr<ID3D12Resource> m_indexBuffer;
			ComPtr<ID3D12Resource> m_indexBufferUpload;
			ComPtr<ID3D12Resource> m_vertexBuffer;
			ComPtr<ID3D12Resource> m_vertexBufferUpload;
			UINT m_rtvDescriptorSize;
			//InputState m_keyboardInput;
			LightData m_lights[APP_NUM_ACTIVE_LIGHTS];
			//Camera m_lightCameras[APP_NUM_ACTIVE_LIGHTS];
			//Camera m_camera;
			StepTimer m_timer;

			// Synchronization objects.
			HANDLE m_workerBeginRenderFrame[APP_NUM_FRAMES_IN_FLIGHT];
			HANDLE m_workerFinishShadowPass[APP_NUM_FRAMES_IN_FLIGHT];
			HANDLE m_workerFinishedRenderFrame[APP_NUM_FRAMES_IN_FLIGHT];
			HANDLE m_threadHandles[APP_NUM_FRAMES_IN_FLIGHT];
			UINT m_frameIndex;
			HANDLE m_fenceEvent;
			ComPtr<ID3D12Fence> m_fence;
			UINT64 m_fenceValue;

			// Singleton object so that worker threads can share members.
			static D3D12Multithreading* s_app;

			// Frame resources.
			FrameResource* m_frameResources[APP_NUM_FRAMES_IN_FLIGHT];
			FrameResource* m_pCurrentFrameResource;
			int m_currentFrameResourceIndex;

			struct ThreadParameter
			{
				int threadIndex;
			};
			ThreadParameter m_threadParameters[APP_NUM_FRAMES_IN_FLIGHT];

			void WorkerThread(int threadIndex);
			void SetCommonPipelineState(ID3D12GraphicsCommandList* pCommandList);

			void LoadPipeline();
			void LoadAssets();
			void RestoreD3DResources();
			void ReleaseD3DResources();
			void WaitForGpu();
			void LoadContexts();
			void BeginFrame();
			void MidFrame();
			void EndFrame();

			// Adapter info.
			bool m_useWarpDevice;
			// Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, *ppAdapter will be set to nullptr.

			void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter)
			{
				*ppAdapter = nullptr;

				ComPtr<IDXGIAdapter1> adapter;

				ComPtr<IDXGIFactory6> factory6;
				if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
				{
					for (
						UINT adapterIndex = 0;
						SUCCEEDED(factory6->EnumAdapterByGpuPreference(
							adapterIndex,
							requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
							IID_PPV_ARGS(&adapter)));
							++adapterIndex)
					{
						DXGI_ADAPTER_DESC1 desc;
						adapter->GetDesc1(&desc);

						if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
						{
							// Don't select the Basic Render Driver adapter.
							// If you want a software adapter, pass in "/warp" on the command line.
							continue;
						}

						// Check to see whether the adapter supports Direct3D 12, but don't create the
						// actual device yet.
						if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
						{
							break;
						}
					}
				}

				if (adapter.Get() == nullptr)
				{
					for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
					{
						DXGI_ADAPTER_DESC1 desc;
						adapter->GetDesc1(&desc);

						if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
						{
							// Don't select the Basic Render Driver adapter.
							// If you want a software adapter, pass in "/warp" on the command line.
							continue;
						}

						// Check to see whether the adapter supports Direct3D 12, but don't create the
						// actual device yet.
						if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
						{
							break;
						}
					}
				}

				*ppAdapter = adapter.Detach();
			}
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

