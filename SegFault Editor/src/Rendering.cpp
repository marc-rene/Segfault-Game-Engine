#include "Rendering.hpp"



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

	for (UINT i = 0; i < APP_NUM_BACK_BUFFERS; i++)
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
	for (UINT i = 0; i < Tarantino::Graphics::APP_NUM_BACK_BUFFERS; i++)
	{
		ID3D12Resource* pBackBuffer = nullptr;
		Tarantino::Graphics::DX12::g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
		Tarantino::Graphics::DX12::g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, Tarantino::Graphics::DX12::g_mainRenderTargetDescriptor[i]);
		Tarantino::Graphics::DX12::g_mainRenderTargetResource[i] = pBackBuffer;
	}
}



void Tarantino::Graphics::DX12::StartFrame()
{
	m_pCurrentFrameResource->Init();

	// Indicate that the back buffer will be used as a render target.
	m_pCurrentFrameResource->m_commandLists[CommandListPre]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the render target and depth stencil.
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	m_pCurrentFrameResource->m_commandLists[CommandListPre]->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_pCurrentFrameResource->m_commandLists[CommandListPre]->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	ThrowIfFailed(m_pCurrentFrameResource->m_commandLists[CommandListPre]->Close());
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
		sd.BufferCount = APP_NUM_BACK_BUFFERS;
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
		desc.NumDescriptors = APP_NUM_BACK_BUFFERS;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 1;
		if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
			return false;

		SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
		for (UINT i = 0; i < APP_NUM_BACK_BUFFERS; i++)
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
		g_pSwapChain->SetMaximumFrameLatency(APP_NUM_BACK_BUFFERS);
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

