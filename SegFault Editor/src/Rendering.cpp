#include "Rendering.hpp"
#include <d3dcompiler.h>



void Tarantino::Graphics::WaitForLastSubmittedFrame()
{
	DX12::FrameContext* frameCtx = &DX12::g_frameContext[DX12::g_frameIndex % APP_NUM_FRAMES_IN_FLIGHT];

	UINT64 fenceValue = frameCtx->FenceValue;
	if (fenceValue == 0)
		return; // No fence was signaled

	frameCtx->FenceValue = 0;
	if (DX12::g_fence->GetCompletedValue() >= fenceValue)
		return;

	DX12::g_fence->SetEventOnCompletion(fenceValue, DX12::g_fenceEvent);
	WaitForSingleObject(DX12::g_fenceEvent, INFINITE);
}



void Tarantino::Graphics::CleanupRenderTarget()
{
	WaitForLastSubmittedFrame();

	for (UINT i = 0; i < APP_NUM_FRAMES_IN_FLIGHT; i++)
	{
		if (DX12::g_mainRenderTargetResource[i])
		{
			DX12::g_mainRenderTargetResource[i]->Release();
			DX12::g_mainRenderTargetResource[i] = nullptr;
		}
	}
}



void Tarantino::Graphics::CreateRenderTarget()
{
	for (UINT i = 0; i < Tarantino::Graphics::APP_NUM_FRAMES_IN_FLIGHT; i++)
	{
		ID3D12Resource* pBackBuffer = nullptr;
		Tarantino::Graphics::DX12::g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
		Tarantino::Graphics::DX12::g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, Tarantino::Graphics::DX12::g_mainRenderTargetDescriptor[i]);
		Tarantino::Graphics::DX12::g_mainRenderTargetResource[i] = pBackBuffer;
	}
}



void Tarantino::Graphics::DX12::StartFrame()
{

}

void Tarantino::Graphics::FinishRenderFrame()
{
	static Tarantino::Graphics::DX12::FrameContext* frameCtx;
	static UINT backBufferIdx;
	static D3D12_RESOURCE_BARRIER barrier = {};
	static const float clear_color_with_alpha[4] = { 0.1f, 0, 0, 1 };
	static HRESULT hr;
	static UINT64 fenceValue;

	frameCtx = DX12::WaitForNextFrameResources();
	backBufferIdx = DX12::g_pSwapChain->GetCurrentBackBufferIndex();
	frameCtx->CommandAllocator->Reset();

	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = DX12::g_mainRenderTargetResource[backBufferIdx];
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	DX12::g_pd3dCommandList->Reset(frameCtx->CommandAllocator, nullptr);
	DX12::g_pd3dCommandList->ResourceBarrier(1, &barrier);

	DX12::g_pd3dCommandList->ClearRenderTargetView(DX12::g_mainRenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, nullptr);
	DX12::g_pd3dCommandList->OMSetRenderTargets(1, &DX12::g_mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
	DX12::g_pd3dCommandList->SetDescriptorHeaps(1, &DX12::g_pd3dSrvDescHeap);

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), DX12::g_pd3dCommandList);

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	DX12::g_pd3dCommandList->ResourceBarrier(1, &barrier);
	DX12::g_pd3dCommandList->Close();

	DX12::g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&DX12::g_pd3dCommandList);

	// Present
	hr = DX12::g_pSwapChain->Present(Settings::r_enable_vsync, 0);   // vsync
	DX12::g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);

	fenceValue = DX12::g_fenceLastSignaledValue + 1;
	DX12::g_pd3dCommandQueue->Signal(DX12::g_fence, fenceValue);
	DX12::g_fenceLastSignaledValue = fenceValue;
	frameCtx->FenceValue = fenceValue;
}



float Tarantino::Graphics::GetDeltaTime_Render()
{
	static LARGE_INTEGER currentTime;
	static LARGE_INTEGER g_Frequency;
	static LARGE_INTEGER g_PrevTime;
	static UINT current_saved_index;
	static float dt;
	static bool need_init = true;

	if (need_init)
	{
		QueryPerformanceFrequency(&g_Frequency);
		QueryPerformanceCounter(&g_PrevTime);
		current_saved_index = Tarantino::Graphics::DX12::g_frameIndex;
		need_init = false;
	}
	if (current_saved_index == Tarantino::Graphics::DX12::g_frameIndex)
		return dt;

	QueryPerformanceCounter(&currentTime);

	dt = static_cast<float>(currentTime.QuadPart - g_PrevTime.QuadPart) / g_Frequency.QuadPart;
	g_PrevTime = currentTime;
	current_saved_index = Tarantino::Graphics::DX12::g_frameIndex;
	return dt;
}

float Tarantino::Graphics::GetFPS_Render()
{
	return 1.0f / Tarantino::Graphics::GetDeltaTime_Render();
}


Tarantino::Graphics::DX12::FrameContext* Tarantino::Graphics::DX12::WaitForNextFrameResources()
{
	UINT nextFrameIndex = g_frameIndex + 1;
	g_frameIndex = nextFrameIndex;

	HANDLE waitableObjects[] = { g_hSwapChainWaitableObject, nullptr };
	DWORD numWaitableObjects = 1;

	FrameContext* frameCtx = &g_frameContext[nextFrameIndex % APP_NUM_FRAMES_IN_FLIGHT];
	UINT64 fenceValue = frameCtx->FenceValue;
	if (fenceValue != 0) // means no fence was signaled
	{
		frameCtx->FenceValue = 0;
		g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
		waitableObjects[1] = g_fenceEvent;
		numWaitableObjects = 2;
	}

	WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

	return frameCtx;
}



void Tarantino::Graphics::DX12::CleanupDeviceD3D()
{
	CleanupRenderTarget();

	if (g_pSwapChain) {
		g_pSwapChain->SetFullscreenState(false, nullptr); g_pSwapChain->Release(); g_pSwapChain = nullptr;
	}

	if (g_hSwapChainWaitableObject != nullptr) {
		CloseHandle(g_hSwapChainWaitableObject);
	}

	for (UINT i = 0; i < APP_NUM_FRAMES_IN_FLIGHT; i++)
		if (g_frameContext[i].CommandAllocator)
		{
			g_frameContext[i].CommandAllocator->Release();
			g_frameContext[i].CommandAllocator = nullptr;
		}

	if (g_pd3dCommandQueue) { g_pd3dCommandQueue->Release(); g_pd3dCommandQueue = nullptr; }
	if (g_pd3dCommandList) { g_pd3dCommandList->Release(); g_pd3dCommandList = nullptr; }
	if (g_pd3dRtvDescHeap) { g_pd3dRtvDescHeap->Release(); g_pd3dRtvDescHeap = nullptr; }
	if (g_pd3dSrvDescHeap) { g_pd3dSrvDescHeap->Release(); g_pd3dSrvDescHeap = nullptr; }
	if (g_fence) { g_fence->Release(); g_fence = nullptr; }
	if (g_fenceEvent) { CloseHandle(g_fenceEvent); g_fenceEvent = nullptr; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }

#ifdef DX12_ENABLE_DEBUG_LAYER
	IDXGIDebug1* pDebug = nullptr;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
	{
		pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
		pDebug->Release();
	}
#endif
}



bool Tarantino::Graphics::DX12::CreateDeviceD3D(HWND hwnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC1 sd;
	{
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = APP_NUM_FRAMES_IN_FLIGHT;
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

	// [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
	ID3D12Debug* pdx12Debug = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
		pdx12Debug->EnableDebugLayer();
#endif

	// Create device
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	if (D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&g_pd3dDevice)) != S_OK)
		return false;

	// [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
	if (pdx12Debug != nullptr)
	{
		ID3D12InfoQueue* pInfoQueue = nullptr;
		g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		pInfoQueue->Release();
		pdx12Debug->Release();
	}
#endif

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.NumDescriptors = APP_NUM_FRAMES_IN_FLIGHT;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 1;
		if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
			return false;

		SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
		for (UINT i = 0; i < APP_NUM_FRAMES_IN_FLIGHT; i++)
		{
			g_mainRenderTargetDescriptor[i] = rtvHandle;
			rtvHandle.ptr += rtvDescriptorSize;
		}
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = APP_SRV_HEAP_SIZE;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
			return false;
		g_pd3dSrvDescHeapAlloc.Create(g_pd3dDevice, g_pd3dSrvDescHeap);
	}

	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 1;
		if (g_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_pd3dCommandQueue)) != S_OK)
			return false;
	}

	for (UINT i = 0; i < APP_NUM_FRAMES_IN_FLIGHT; i++)
		if (g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_frameContext[i].CommandAllocator)) != S_OK)
			return false;

	if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[0].CommandAllocator, nullptr, IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK ||
		g_pd3dCommandList->Close() != S_OK)
		return false;

	if (g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)) != S_OK)
		return false;

	g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (g_fenceEvent == nullptr)
		return false;

	{
		IDXGIFactory4* dxgiFactory = nullptr;
		IDXGISwapChain1* swapChain1 = nullptr;
		if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
			return false;
		if (dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, hwnd, &sd, nullptr, nullptr, &swapChain1) != S_OK)
			return false;
		if (swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain)) != S_OK)
			return false;
		swapChain1->Release();
		dxgiFactory->Release();
		g_pSwapChain->SetMaximumFrameLatency(APP_NUM_FRAMES_IN_FLIGHT);
		g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
	}

	CreateRenderTarget();
	return true;
}



HWND Tarantino::Window::GetSDLMainWindowHandle()
{
	HWND window_handle = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(SDL_MainWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
	if (window_handle == nullptr)
		WARNc("SDL_GetPointerProperty is returning NullPtr... just fyi");

	return window_handle;
}



bool Tarantino::Window::SDL_Window_isValid()
{
	if (SDL_MainWindow == nullptr)
	{
		OhSHITc("Our SDL Window is banjaxxed! \n{}\n", SDL_GetError());
		return false;
	}

	return true;
}



// ----------------------------------------------------------------------------
// ATTEMPT 2
// ----------------------------------------------------------------------------



/*


Tarantino::Graphics::D3D12Multithreading::D3D12Multithreading(UINT width, UINT height, std::wstring name) :
	m_frameIndex(0),
	m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
	m_fenceValue(0),
	m_rtvDescriptorSize(0),
	m_currentFrameResourceIndex(0),
	m_pCurrentFrameResource(nullptr)
{
	s_app = this;

	DXGIDeclareAdapterRemovalSupport();
}



void Tarantino::Graphics::D3D12Multithreading::OnInit()
{
	LoadPipeline();
	LoadAssets();
	LoadContexts();
}

// Load the rendering pipeline dependencies.
void Tarantino::Graphics::D3D12Multithreading::LoadPipeline()
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	ComPtr<IDXGIFactory4> factory;
	CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));

	if (m_useWarpDevice)
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

		D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		);
	}
	else
	{
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(factory.Get(), &hardwareAdapter, true);

		D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		);
	}

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = APP_NUM_FRAMES_IN_FLIGHT;
	swapChainDesc.Width = 1600; //TODO: get window dimensions
	swapChainDesc.Height = 900;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
		Window::GetSDLMainWindowHandle(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	);
	swapChain.As(&m_swapChain);
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = APP_NUM_FRAMES_IN_FLIGHT;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));

		// Describe and create a depth stencil view (DSV) descriptor heap.
		// Each frame has its own depth stencils (to write shadows onto) 
		// and then there is one for the scene itself.
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1 + APP_NUM_FRAMES_IN_FLIGHT * 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));

		// Describe and create a shader resource view (SRV) and constant 
		// buffer view (CBV) descriptor heap.  Heap layout: null views, 
		// object diffuse + normal textures views, frame 1's shadow buffer, 
		// frame 1's 2x constant buffer, frame 2's shadow buffer, frame 2's 
		// 2x constant buffers, etc...
		const UINT nullSrvCount = 2;        // Null descriptors are needed for out of bounds behavior reads.
		const UINT cbvCount = APP_NUM_FRAMES_IN_FLIGHT * 2;
		const UINT srvCount = 16; // TODO: add in proper textire init
		D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc = {};
		cbvSrvHeapDesc.NumDescriptors = nullSrvCount + cbvCount + srvCount;
		cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		m_device->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_PPV_ARGS(&m_cbvSrvHeap));

		// Describe and create a sampler descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
		samplerHeapDesc.NumDescriptors = 2;        // One clamp and one wrap sampler.
		samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		m_device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_samplerHeap));

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
}

// Load the sample assets.
void Tarantino::Graphics::D3D12Multithreading::LoadAssets()
{
	// Create the root signature.
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_DESCRIPTOR_RANGE1 ranges[4]; // Perfomance TIP: Order from most frequent to least frequent.
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);    // 2 frequently changed diffuse + normal textures - using registers t1 and t2.
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);    // 1 frequently changed constant buffer.
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);                                                // 1 infrequently changed shadow texture - starting in register t0.
		ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 2, 0);                                            // 2 static samplers.

		CD3DX12_ROOT_PARAMETER1 rootParameters[4];
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[3].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_PIXEL);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error);
		m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
	}

	// Create the pipeline state, which includes loading shaders.
	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

		D3DCompileFromFile(L"Shaders/shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
		D3DCompileFromFile(L"Shaders/shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);

		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
		//inputLayoutDesc.pInputElementDescs = SampleAssets::StandardVertexDescription;
		//inputLayoutDesc.NumElements = _countof(SampleAssets::StandardVertexDescription);

		CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		depthStencilDesc.StencilEnable = FALSE;

		// Describe and create the PSO for rendering the scene.
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = inputLayoutDesc;
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = depthStencilDesc;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = 1;

		m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));

		// Alter the description and create the PSO for rendering
		// the shadow map.  The shadow map does not use a pixel
		// shader or render targets.
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(0, 0);
		psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
		psoDesc.NumRenderTargets = 0;

		m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStateShadowMap));
	}

	// Create temporary command list for initial GPU setup.
	ComPtr<ID3D12GraphicsCommandList> commandList;
	m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&commandList));

	// Create render target views (RTVs).
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < APP_NUM_FRAMES_IN_FLIGHT; i++)
	{
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
		m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}

	// Create the depth stencil.
	{
		CD3DX12_RESOURCE_DESC shadowTextureDesc(
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			0,
			static_cast<UINT>(m_viewport.Width),
			static_cast<UINT>(m_viewport.Height),
			1,
			1,
			DXGI_FORMAT_D32_FLOAT,
			1,
			0,
			D3D12_TEXTURE_LAYOUT_UNKNOWN,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);

		D3D12_CLEAR_VALUE clearValue;    // Performance tip: Tell the runtime at resource creation the desired clear value.
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		CD3DX12_HEAP_PROPERTIES hprops;
		m_device->CreateCommittedResource(
			&hprops,
			D3D12_HEAP_FLAG_NONE,
			&shadowTextureDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(&m_depthStencil));


		// Create the depth stencil view.
		m_device->CreateDepthStencilView(m_depthStencil.Get(), nullptr, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	// Load scene assets.
	UINT fileSize = 0;
	UINT8* pAssetData;
	//ReadDataFromFile(GetAssetFullPath(SampleAssets::DataFileName).c_str(), &pAssetData, &fileSize);

	// Create the vertex buffer.
	{
		CD3DX12_HEAP_PROPERTIES hprops;
		m_device->CreateCommittedResource(
			&hprops,
			D3D12_HEAP_FLAG_NONE,
			nullptr, // TODO: put in proper vertex buffer inits
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer));


		{
			m_device->CreateCommittedResource(
				&hprops,
				D3D12_HEAP_FLAG_NONE,
				nullptr, // TODO: put in proper vertex buffer inits
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_vertexBufferUpload));

			// Copy data to the upload heap and then schedule a copy 
			// from the upload heap to the vertex buffer.
			D3D12_SUBRESOURCE_DATA vertexData = {};
			vertexData.pData = pAssetData + 512; // TODO: put in proper vertex buffer inits
			vertexData.RowPitch = 512;
			vertexData.SlicePitch = vertexData.RowPitch;

			UpdateSubresources<1>(commandList.Get(), m_vertexBuffer.Get(), m_vertexBufferUpload.Get(), 0, 0, 1, &vertexData);
			//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		}

		// Initialize the vertex buffer view.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.SizeInBytes = 512; // TODO: put in proper vertex buffer inits
		m_vertexBufferView.StrideInBytes = 512; // TODO: put in proper vertex buffer inits
	}

	// Create the index buffer.
	{
		CD3DX12_HEAP_PROPERTIES hprops;
		m_device->CreateCommittedResource(
			&hprops,
			D3D12_HEAP_FLAG_NONE,
			nullptr, // TODO: put in proper vertex buffer inits
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_indexBuffer));
		{
			m_device->CreateCommittedResource(
				&hprops,
				D3D12_HEAP_FLAG_NONE,
				nullptr,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_indexBufferUpload));

			// Copy data to the upload heap and then schedule a copy 
			// from the upload heap to the index buffer.
			D3D12_SUBRESOURCE_DATA indexData = {};
			indexData.pData = pAssetData + 512;
			indexData.RowPitch = 512;
			indexData.SlicePitch = indexData.RowPitch;

			UpdateSubresources<1>(commandList.Get(), m_indexBuffer.Get(), m_indexBufferUpload.Get(), 0, 0, 1, &indexData);
			//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

		}

		// Initialize the index buffer view.
		m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		m_indexBufferView.SizeInBytes = 512;  // TODO: put in proper vertex buffer inits
		m_indexBufferView.Format = DXGI_FORMAT_R32_UINT; // TODO: put in proper vertex buffer inits
	}

	// Create shader resources.
	{
		// Get the CBV SRV descriptor size for the current device.
		const UINT cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Get a handle to the start of the descriptor heap.
		CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvHandle(m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart());

		{
			// Describe and create 2 null SRVs. Null descriptors are needed in order 
			// to achieve the effect of an "unbound" resource.
			D3D12_SHADER_RESOURCE_VIEW_DESC nullSrvDesc = {};
			nullSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			nullSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			nullSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			nullSrvDesc.Texture2D.MipLevels = 1;
			nullSrvDesc.Texture2D.MostDetailedMip = 0;
			nullSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

			m_device->CreateShaderResourceView(nullptr, &nullSrvDesc, cbvSrvHandle);
			cbvSrvHandle.Offset(cbvSrvDescriptorSize);

			m_device->CreateShaderResourceView(nullptr, &nullSrvDesc, cbvSrvHandle);
			cbvSrvHandle.Offset(cbvSrvDescriptorSize);
		}

		// TODO: put in proper Texture buffer inits



	}

	free(pAssetData);

	// Create the samplers.
	{
		// Get the sampler descriptor size for the current device.
		const UINT samplerDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		// Get a handle to the start of the descriptor heap.
		CD3DX12_CPU_DESCRIPTOR_HANDLE samplerHandle(m_samplerHeap->GetCPUDescriptorHandleForHeapStart());

		// Describe and create the wrapping sampler, which is used for 
		// sampling diffuse/normal maps.
		D3D12_SAMPLER_DESC wrapSamplerDesc = {};
		wrapSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		wrapSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		wrapSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		wrapSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		wrapSamplerDesc.MinLOD = 0;
		wrapSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		wrapSamplerDesc.MipLODBias = 0.0f;
		wrapSamplerDesc.MaxAnisotropy = 1;
		wrapSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		wrapSamplerDesc.BorderColor[0] = wrapSamplerDesc.BorderColor[1] = wrapSamplerDesc.BorderColor[2] = wrapSamplerDesc.BorderColor[3] = 0;
		m_device->CreateSampler(&wrapSamplerDesc, samplerHandle);

		// Move the handle to the next slot in the descriptor heap.
		samplerHandle.Offset(samplerDescriptorSize);

		// Describe and create the point clamping sampler, which is 
		// used for the shadow map.
		D3D12_SAMPLER_DESC clampSamplerDesc = {};
		clampSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		clampSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		clampSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		clampSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		clampSamplerDesc.MipLODBias = 0.0f;
		clampSamplerDesc.MaxAnisotropy = 1;
		clampSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		clampSamplerDesc.BorderColor[0] = clampSamplerDesc.BorderColor[1] = clampSamplerDesc.BorderColor[2] = clampSamplerDesc.BorderColor[3] = 0;
		clampSamplerDesc.MinLOD = 0;
		clampSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		m_device->CreateSampler(&clampSamplerDesc, samplerHandle);
	}

	// Create lights.
	for (int i = 0; i < APP_NUM_ACTIVE_LIGHTS; i++)
	{
		// Set up each of the light positions and directions (they all start 
		// in the same place).
		m_lights[i].position = { 0.0f, 15.0f, -30.0f, 1.0f };
		m_lights[i].direction = { 0.0, 0.0f, 1.0f, 0.0f };
		m_lights[i].falloff = { 800.0f, 1.0f, 0.0f, 1.0f };
		m_lights[i].color = { 0.7f, 0.7f, 0.7f, 1.0f };

		DirectX::XMVECTOR eye = DirectX::XMLoadFloat4(&m_lights[i].position);
		DirectX::XMVECTOR at = DirectX::XMVectorAdd(eye, XMLoadFloat4(&m_lights[i].direction));
		DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		m_lightCameras[i].Set(eye, at, up);
	}

	// Close the command list and use it to execute the initial GPU setup.
	commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Create frame resources.
	for (int i = 0; i < APP_NUM_FRAMES_IN_FLIGHT; i++)
	{
		m_frameResources[i] = new FrameResource(m_device.Get(), m_pipelineState.Get(), m_pipelineStateShadowMap.Get(), m_dsvHeap.Get(), m_cbvSrvHeap.Get(), &m_viewport, i);
		m_frameResources[i]->WriteConstantBuffers(&m_viewport, &m_camera, m_lightCameras, m_lights);
	}
	m_currentFrameResourceIndex = 0;
	m_pCurrentFrameResource = m_frameResources[m_currentFrameResourceIndex];

	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		m_device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
		m_fenceValue++;

		// Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			HRESULT_FROM_WIN32(GetLastError());
		}

		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.

		// Signal and increment the fence value.
		const UINT64 fenceToWaitFor = m_fenceValue;
		m_commandQueue->Signal(m_fence.Get(), fenceToWaitFor);
		m_fenceValue++;

		// Wait until the fence is completed.
		m_fence->SetEventOnCompletion(fenceToWaitFor, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}

// Initialize threads and events.
void Tarantino::Graphics::D3D12Multithreading::LoadContexts()
{
	struct threadwrapper
	{
		static unsigned int WINAPI thunk(LPVOID lpParameter)
		{
			ThreadParameter* parameter = reinterpret_cast<ThreadParameter*>(lpParameter);
			D3D12Multithreading::Get()->WorkerThread(parameter->threadIndex);
			return 0;
		}
	};

	for (int i = 0; i < APP_NUM_FRAMES_IN_FLIGHT; i++)
	{
		m_workerBeginRenderFrame[i] = CreateEvent(
			NULL,
			FALSE,
			FALSE,
			NULL);

		m_workerFinishedRenderFrame[i] = CreateEvent(
			NULL,
			FALSE,
			FALSE,
			NULL);

		m_workerFinishShadowPass[i] = CreateEvent(
			NULL,
			FALSE,
			FALSE,
			NULL);

		m_threadParameters[i].threadIndex = i;

		m_threadHandles[i] = reinterpret_cast<HANDLE>(_beginthreadex(
			nullptr,
			0,
			threadwrapper::thunk,
			reinterpret_cast<LPVOID>(&m_threadParameters[i]),
			0,
			nullptr));

		assert(m_workerBeginRenderFrame[i] != NULL);
		assert(m_workerFinishedRenderFrame[i] != NULL);
		assert(m_threadHandles[i] != NULL);

	}
}

// Update frame-based values.
void Tarantino::Graphics::D3D12Multithreading::OnUpdate()
{
	// Get current GPU progress against submitted workload. Resources still scheduled 
	// for GPU execution cannot be modified or else undefined behavior will result.
	const UINT64 lastCompletedFence = m_fence->GetCompletedValue();

	// Move to the next frame resource.
	m_currentFrameResourceIndex = (m_currentFrameResourceIndex + 1) % APP_NUM_FRAMES_IN_FLIGHT;
	m_pCurrentFrameResource = m_frameResources[m_currentFrameResourceIndex];

	// Make sure that this frame resource isn't still in use by the GPU.
	// If it is, wait for it to complete.
	if (m_pCurrentFrameResource->m_fenceValue > lastCompletedFence)
	{
		HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (eventHandle == nullptr)
		{
			HRESULT_FROM_WIN32(GetLastError());
		}
		m_fence->SetEventOnCompletion(m_pCurrentFrameResource->m_fenceValue, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	//float frameTime = static_cast<float>(m_timer.GetElapsedSeconds());
	//float frameChange = 2.0f * frameTime;
	
	//if (m_keyboardInput.animate)
	//{
	//	for (int i = 0; i < NumLights; i++)
	//	{
	//		float direction = frameChange * pow(-1.0f, i);
	//		XMStoreFloat4(&m_lights[i].position, XMVector4Transform(XMLoadFloat4(&m_lights[i].position), XMMatrixRotationY(direction)));
	//
	//		XMVECTOR eye = XMLoadFloat4(&m_lights[i].position);
	//		XMVECTOR at = XMVectorSet(0.0f, 8.0f, 0.0f, 0.0f);
	//		XMStoreFloat4(&m_lights[i].direction, XMVector3Normalize(XMVectorSubtract(at, eye)));
	//		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	//		m_lightCameras[i].Set(eye, at, up);
	//
	//		m_lightCameras[i].Get3DViewProjMatrices(&m_lights[i].view, &m_lights[i].projection, 90.0f, static_cast<float>(m_width), static_cast<float>(m_height));
	//	}
	//}
	

	m_pCurrentFrameResource->WriteConstantBuffers(&m_viewport, &m_camera, m_lightCameras, m_lights);
}




// Render the scene.
void Tarantino::Graphics::D3D12Multithreading::OnRender()
{
	try
	{
		BeginFrame();

		for (int i = 0; i < APP_NUM_FRAMES_IN_FLIGHT; i++)
		{
			SetEvent(m_workerBeginRenderFrame[i]); // Tell each worker to start drawing.
		}

		MidFrame();
		EndFrame();

		WaitForMultipleObjects(APP_NUM_FRAMES_IN_FLIGHT, m_workerFinishShadowPass, TRUE, INFINITE);

		// You can execute command lists on any thread. Depending on the work 
		// load, apps can choose between using ExecuteCommandLists on one thread 
		// vs ExecuteCommandList from multiple threads.
		m_commandQueue->ExecuteCommandLists(APP_NUM_FRAMES_IN_FLIGHT + 2, m_pCurrentFrameResource->m_batchSubmit); // Submit PRE, MID and shadows.

		WaitForMultipleObjects(APP_NUM_FRAMES_IN_FLIGHT, m_workerFinishedRenderFrame, TRUE, INFINITE);

		// Submit remaining command lists.
		m_commandQueue->ExecuteCommandLists(_countof(m_pCurrentFrameResource->m_batchSubmit) - APP_NUM_FRAMES_IN_FLIGHT - 2, m_pCurrentFrameResource->m_batchSubmit + APP_NUM_FRAMES_IN_FLIGHT + 2);


		// Present and update the frame index for the next frame.
		m_swapChain->Present(1, 0);
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		// Signal and increment the fence value.
		m_pCurrentFrameResource->m_fenceValue = m_fenceValue;
		m_commandQueue->Signal(m_fence.Get(), m_fenceValue);
		m_fenceValue++;
	}
	catch (HrException& e)
	{
		if (e.Error() == DXGI_ERROR_DEVICE_REMOVED || e.Error() == DXGI_ERROR_DEVICE_RESET)
		{
			RestoreD3DResources();
		}
		else
		{
			throw;
		}
	}
}


// Release sample's D3D objects.
void Tarantino::Graphics::D3D12Multithreading::ReleaseD3DResources()
{
	m_fence.Reset();
	for (auto& i : m_renderTargets)
	{
		i.Reset();
	}
	m_commandQueue.Reset();
	m_swapChain.Reset();
	m_device.Reset();
}

// Tears down D3D resources and reinitializes them.
void Tarantino::Graphics::D3D12Multithreading::RestoreD3DResources()
{
	// Give GPU a chance to finish its execution in progress.

	WaitForGpu();
	ReleaseD3DResources();
	OnInit();
}

// Wait for pending GPU work to complete.
void Tarantino::Graphics::D3D12Multithreading::WaitForGpu()
{
	// Schedule a Signal command in the queue.
	m_commandQueue->Signal(m_fence.Get(), m_fenceValue);

	// Wait until the fence has been processed.
	m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
}

void Tarantino::Graphics::D3D12Multithreading::OnDestroy()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	{
		const UINT64 fence = m_fenceValue;
		const UINT64 lastCompletedFence = m_fence->GetCompletedValue();

		// Signal and increment the fence value.
		m_commandQueue->Signal(m_fence.Get(), m_fenceValue);
		m_fenceValue++;

		// Wait until the previous frame is finished.
		if (lastCompletedFence < fence)
		{
			m_fence->SetEventOnCompletion(fence, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
		CloseHandle(m_fenceEvent);
	}

	// Close thread events and thread handles.
	for (int i = 0; i < APP_NUM_FRAMES_IN_FLIGHT; i++)
	{
		CloseHandle(m_workerBeginRenderFrame[i]);
		CloseHandle(m_workerFinishShadowPass[i]);
		CloseHandle(m_workerFinishedRenderFrame[i]);
		CloseHandle(m_threadHandles[i]);
	}

	for (int i = 0; i < _countof(m_frameResources); i++)
	{
		delete m_frameResources[i];
	}
}



// Assemble the CommandListPre command list.
void Tarantino::Graphics::D3D12Multithreading::BeginFrame()
{
	m_pCurrentFrameResource->Init();

	// Indicate that the back buffer will be used as a render target.
	//m_pCurrentFrameResource->m_commandLists[CommandListPre]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the render target and depth stencil.
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	m_pCurrentFrameResource->m_commandLists[CommandListPre]->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_pCurrentFrameResource->m_commandLists[CommandListPre]->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	m_pCurrentFrameResource->m_commandLists[CommandListPre]->Close();
}

// Assemble the CommandListMid command list.
void Tarantino::Graphics::D3D12Multithreading::MidFrame()
{
	// Transition our shadow map from the shadow pass to readable in the scene pass.
	m_pCurrentFrameResource->SwapBarriers();

	m_pCurrentFrameResource->m_commandLists[CommandListMid]->Close();
}

// Assemble the CommandListPost command list.
void Tarantino::Graphics::D3D12Multithreading::EndFrame()
{
	m_pCurrentFrameResource->Finish();

	// Indicate that the back buffer will now be used to present.
	//m_pCurrentFrameResource->m_commandLists[CommandListPost]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	m_pCurrentFrameResource->m_commandLists[CommandListPost]->Close();
}

// Worker thread body
// describing the worker's thread index.
void Tarantino::Graphics::D3D12Multithreading::WorkerThread(int threadIndex)
{
	assert(threadIndex >= 0);
	assert(threadIndex < APP_NUM_FRAMES_IN_FLIGHT);


	while (threadIndex >= 0 && threadIndex < APP_NUM_FRAMES_IN_FLIGHT)
	{
		// Wait for main thread to tell us to draw.

		WaitForSingleObject(m_workerBeginRenderFrame[threadIndex], INFINITE);

		ID3D12GraphicsCommandList* pShadowCommandList = m_pCurrentFrameResource->m_shadowCommandLists[threadIndex].Get();
		ID3D12GraphicsCommandList* pSceneCommandList = m_pCurrentFrameResource->m_sceneCommandLists[threadIndex].Get();

		//
		// Shadow pass
		//

		// Populate the command list.
		SetCommonPipelineState(pShadowCommandList);
		m_pCurrentFrameResource->Bind(pShadowCommandList, FALSE, nullptr, nullptr);    // No need to pass RTV or DSV descriptor heap.

		// Set null SRVs for the diffuse/normal textures.
		pShadowCommandList->SetGraphicsRootDescriptorTable(0, m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart());

		// Distribute objects over threads by drawing only 1/NumContexts 
		// objects per worker (i.e. every object such that objectnum % 
		// NumContexts == threadIndex).

		pShadowCommandList->Close();

		// Submit shadow pass.
		SetEvent(m_workerFinishShadowPass[threadIndex]);

		// TODO: Figure out how to do a scene pass

		// Populate the command list.  These can only be sent after the shadow 
		// passes for this frame have been submitted.
		SetCommonPipelineState(pSceneCommandList);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		m_pCurrentFrameResource->Bind(pSceneCommandList, TRUE, &rtvHandle, &dsvHandle);

		D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvHeapStart = m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart();
		const UINT cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		const UINT nullSrvCount = 2;
		//for (int j = threadIndex; j < _countof(SampleAssets::Draws); j += APP_NUM_FRAMES_IN_FLIGHT)
		//{
			// Set the diffuse and normal textures for the current object.
			//CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvHandle(cbvSrvHeapStart, nullSrvCount + drawArgs.DiffuseTextureIndex, cbvSrvDescriptorSize);
			//pSceneCommandList->SetGraphicsRootDescriptorTable(0, cbvSrvHandle);
			//pSceneCommandList->DrawIndexedInstanced(drawArgs.IndexCount, 1, drawArgs.IndexStart, drawArgs.VertexBase, 0);
		//}

		pSceneCommandList->Close();

		// Tell main thread that we are done.
		SetEvent(m_workerFinishedRenderFrame[threadIndex]);
	}

}

void Tarantino::Graphics::D3D12Multithreading::SetCommonPipelineState(ID3D12GraphicsCommandList* pCommandList)
{
	pCommandList->SetGraphicsRootSignature(m_rootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvHeap.Get(), m_samplerHeap.Get() };
	pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	pCommandList->RSSetViewports(1, &m_viewport);
	pCommandList->RSSetScissorRects(1, &m_scissorRect);
	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	pCommandList->IASetIndexBuffer(&m_indexBufferView);
	pCommandList->SetGraphicsRootDescriptorTable(3, m_samplerHeap->GetGPUDescriptorHandleForHeapStart());
	pCommandList->OMSetStencilRef(0);

	// Render targets and depth stencil are set elsewhere because the 
	// depth stencil depends on the frame resource being used.

	// Constant buffers are set elsewhere because they depend on the 
	// frame resource being used.

	// SRVs are set elsewhere because they change based on the object 
	// being drawn.
}
*/