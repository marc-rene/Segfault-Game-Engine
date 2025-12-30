#include "Rendering.hpp"

#include "../Platform/Window.hpp"
#include "../Resources/EngineSettings.hpp"

#include "../Utilities/Editor.hpp"

#define RENDERMASTER_NAME "ENGINE::Rendering::RenderMaster"



bool ENGINE::Rendering::RenderMaster::Initialise()
{
  

    INFO(RENDERMASTER_NAME, "Making DirectX 11 Device + Context");
    // Create D3D11 Device and Context
    
        ID3D11Device* baseDevice;
        ID3D11DeviceContext* baseDeviceContext;
        D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if DEBUG
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        HRESULT hResult = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE,
            0, creationFlags,
            featureLevels, ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION, &baseDevice,
            0, &baseDeviceContext);

        if (FAILED(hResult)) {
            MessageBoxA(0, "D3D11CreateDevice() failed", "Fatal Error", MB_OK);
            CRITICAL("DIRECTX 11 FAILED BECAUSE: {}", GetLastError());
        }

        // Get 1.1 interface of D3D11 Device and Context
        hResult = baseDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)&device_ptr);
        if (FAILED(hResult))
            CRITICAL("DIRECTX 11 ERROR! baseDevice->QueryInterface Failed because: {} ", hResult);

        assert(SUCCEEDED(hResult));
        baseDevice->Release();

        hResult = baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&device_context_ptr);
        assert(SUCCEEDED(hResult));
        baseDeviceContext->Release();
    

#if DEBUG
    // Set up debug layer to break on D3D11 errors
    ID3D11Debug* d3dDebug = nullptr;
    device_ptr->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug);
    if (d3dDebug)
    {
        ID3D11InfoQueue* d3dInfoQueue = nullptr;
        if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
        {
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
            d3dInfoQueue->Release();
        }
        d3dDebug->Release();
    }
#endif



    // Create Swap Chain
    INFO(RENDERMASTER_NAME, "Making Swap-Chain");

    // Get DXGI Factory (needed to create Swap Chain)
    IDXGIFactory2* dxgiFactory;
    {
        IDXGIDevice1* dxgiDevice;
        HRESULT hResult = device_ptr->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice);
        assert(SUCCEEDED(hResult));

        IDXGIAdapter* dxgiAdapter;
        hResult = dxgiDevice->GetAdapter(&dxgiAdapter);
        assert(SUCCEEDED(hResult));
        dxgiDevice->Release();

        DXGI_ADAPTER_DESC adapterDesc;
        dxgiAdapter->GetDesc(&adapterDesc);

        OutputDebugStringA("Graphics Device: ");
        OutputDebugStringW(adapterDesc.Description);

        hResult = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory);
        assert(SUCCEEDED(hResult));
        dxgiAdapter->Release();
    }

    DXGI_SWAP_CHAIN_DESC1 d3d11SwapChainDesc = {};
    d3d11SwapChainDesc.Width = 0; // use window width
    d3d11SwapChainDesc.Height = 0; // use window height
    d3d11SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    d3d11SwapChainDesc.SampleDesc.Count = 1;
    d3d11SwapChainDesc.SampleDesc.Quality = 0;
    d3d11SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    d3d11SwapChainDesc.BufferCount = 2;
    d3d11SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    d3d11SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    d3d11SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    d3d11SwapChainDesc.Flags = 0;

    hResult = dxgiFactory->CreateSwapChainForHwnd(device_ptr, 
        ENGINE::Platform::WindowManager::GetInstance()->GetMainWindowHWND(), 
        &d3d11SwapChainDesc, 0, 0, &swap_chain_ptr);

    assert(SUCCEEDED(hResult));
    dxgiFactory->Release();



    // Create Framebuffer Render Target
    INFO(RENDERMASTER_NAME, "Making Render Target");

    ID3D11Texture2D* d3d11FrameBuffer;
    hResult = swap_chain_ptr->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d11FrameBuffer);
    assert(SUCCEEDED(hResult));

    hResult = device_ptr->CreateRenderTargetView(d3d11FrameBuffer, 0, &render_target_view_ptr);
    assert(SUCCEEDED(hResult));
    d3d11FrameBuffer->Release();
    
    FenceValue = 0;
    return true;
}



void ENGINE::Rendering::RenderMaster::ShowCompletedFrame()
{
    static mint* enable_vysnc = ENGINE::Settings::ActiveSettings::GetSetting_mint(ENGINE::Settings::E_Settings::VIDEO_ENABLE_VSYNC);
    static bool b_EditorEnabled;
    b_EditorEnabled = ENGINE::Editor::Is_Initialised();
    if (b_EditorEnabled)
        ENGINE::Editor::GetInstance()->ComputeDrawData();
    
    device_context_ptr->OMSetRenderTargets(1, &render_target_view_ptr, nullptr);
    ClearRenderTarget();

    if (b_EditorEnabled)
        ENGINE::Editor::GetInstance()->RenderDrawData();

    swap_chain_ptr->Present(1, 0);
}



long ENGINE::Rendering::RenderMaster::ResizeSwapChainBuffer()
{
    device_context_ptr->OMSetRenderTargets(0, 0, 0);
    render_target_view_ptr->Release();

    HRESULT res = swap_chain_ptr->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    assert(SUCCEEDED(res));

    ID3D11Texture2D* d3d11FrameBuffer;
    res = swap_chain_ptr->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d11FrameBuffer);
    assert(SUCCEEDED(res));

    res = device_ptr->CreateRenderTargetView(d3d11FrameBuffer, NULL, &render_target_view_ptr);
    assert(SUCCEEDED(res));
    d3d11FrameBuffer->Release();

    return res;
}

long ENGINE::Rendering::RenderMaster::CompileVertexShaderFromFile(std::string ShaderFileName)
{
    ID3DBlob* shaderCompileErrorsBlob;
    std::wstring L_ShaderFileName = std::wstring(ShaderFileName.begin(), ShaderFileName.end());
    HRESULT hResult = D3DCompileFromFile(L_ShaderFileName.c_str(), nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &VertexShaderBlob, &shaderCompileErrorsBlob);
    
    if (FAILED(hResult))
    {
        const char* errorString = NULL;
        if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            errorString = "Could not compile shader; file not found";
            OhSHIT(RENDERMASTER_NAME, "Directx 11 error: {}", errorString);
        }
        else if (shaderCompileErrorsBlob) {
            errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
            OhSHIT(RENDERMASTER_NAME, "Directx 11 error: {}", errorString);
            shaderCompileErrorsBlob->Release();
        }
        MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
        return NULL;
    }

    hResult = device_ptr->CreateVertexShader(VertexShaderBlob->GetBufferPointer(), VertexShaderBlob->GetBufferSize(), nullptr, &PrimaryVertexShader);
    assert(SUCCEEDED(hResult));
    
}



long ENGINE::Rendering::RenderMaster::CompilePixelShaderFromFile(std::string ShaderFileName)
{
    ID3DBlob* psBlob;
    ID3DBlob* shaderCompileErrorsBlob;
    std::wstring L_ShaderFileName = std::wstring(ShaderFileName.begin(), ShaderFileName.end());
    HRESULT hResult = D3DCompileFromFile(L_ShaderFileName.c_str(), nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &psBlob, &shaderCompileErrorsBlob);
    if (FAILED(hResult))
    {
        const char* errorString = NULL;
        if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            errorString = "Could not compile shader; file not found";
            OhSHIT(RENDERMASTER_NAME, "Directx 11 error: {}", errorString);
        }
        else if (shaderCompileErrorsBlob) {
            errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
            OhSHIT(RENDERMASTER_NAME, "Directx 11 error: {}", errorString);
            shaderCompileErrorsBlob->Release();
        }
        MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    hResult = device_ptr->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &PrimaryPixelShader);
    assert(SUCCEEDED(hResult));
    psBlob->Release();

    return hResult;
}


