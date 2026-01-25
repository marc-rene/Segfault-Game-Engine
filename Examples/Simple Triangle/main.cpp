#include "Apocalypse.hpp"

#include <d3dcompiler.h>
#include <d3dx12.h>

#include "../../Engine/ThirdParty/DirectX_Headers/include/directx/d3dx12_root_signature.h"

using namespace DirectX;

// Vertex data for a colored cube.
struct VertexPosColor
{
    XMFLOAT3 Position;
    XMFLOAT3 Color;
};

ComPtr<ID3D12Resource> Cube_VertexBuffer;
D3D12_VERTEX_BUFFER_VIEW Cube_VertexBufferView;

ComPtr<ID3D12Resource> Cube_IndexBuffer;
D3D12_INDEX_BUFFER_VIEW Cube_IndexBufferView;


// Depth buffer.
ComPtr<ID3D12Resource> m_DepthBuffer;
// Descriptor heap for depth buffer.
ComPtr<ID3D12DescriptorHeap> m_DSVHeap;

// Root signature
ComPtr<ID3D12RootSignature> m_RootSignature;

// Pipeline state object.
ComPtr<ID3D12PipelineState> m_PipelineState;


DirectX::XMMATRIX m_ModelMatrix;
DirectX::XMMATRIX m_ViewMatrix;
DirectX::XMMATRIX m_ProjectionMatrix;


// All of the points that make up our cube
static VertexPosColor Cube_Vertexes[8] = {
    {XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f)}, // 0
    {XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)}, // 1
    {XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f)}, // 2
    {XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f)}, // 3
    {XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f)}, // 4
    {XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f)}, // 5
    {XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f)}, // 6
    {XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f)} // 7
};

// How to draw our cube and in what order
static WORD Cube_Indicies[36] =
{
    0, 1, 2, 0, 2, 3,
    4, 6, 5, 4, 7, 6,
    4, 5, 1, 4, 1, 0,
    3, 2, 6, 3, 6, 7,
    1, 5, 6, 1, 6, 2,
    4, 0, 3, 4, 3, 7
};

bool m_ContentLoaded = false;

void ResizeDepthBuffer(ENGINE::RUNTIME::ClientRuntime* runtime, int width, int height)
{
    if (m_ContentLoaded)
    {
        // Flush any GPU commands that might be referencing the depth buffer.
        runtime->Get_DaVinci_instance()->Get_Command_Queue(D3D12_COMMAND_LIST_TYPE_DIRECT)->Flush();
        runtime->Get_DaVinci_instance()->Get_Command_Queue(D3D12_COMMAND_LIST_TYPE_COMPUTE)->Flush();
        runtime->Get_DaVinci_instance()->Get_Command_Queue(D3D12_COMMAND_LIST_TYPE_COPY)->Flush();

        width = std::max(1, width);
        height = std::max(1, height);

        auto device = runtime->Get_DaVinci_instance()->Get_Active_Device();

        // Resize screen dependent resources.
        // Create a depth buffer.
        D3D12_CLEAR_VALUE optimizedClearValue = {};
        optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        optimizedClearValue.DepthStencil = {1.0f, 0};

        auto new_heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto heap_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0,
                                                      D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
        device->CreateCommittedResource(
            &new_heap,
            D3D12_HEAP_FLAG_NONE,
            &heap_desc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &optimizedClearValue,
            IID_PPV_ARGS(&m_DepthBuffer));

        // Update the depth-stencil view.
        D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
        dsv.Format = DXGI_FORMAT_D32_FLOAT;
        dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsv.Texture2D.MipSlice = 0;
        dsv.Flags = D3D12_DSV_FLAG_NONE;

        device->CreateDepthStencilView(m_DepthBuffer.Get(), &dsv,
                                       m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
    }
}

bool load_mesh(ENGINE::RUNTIME::ClientRuntime* runtime)
{
    auto device = runtime->Get_DaVinci_instance()->Get_Active_Device();
    auto command_queue = runtime->Get_DaVinci_instance()->Get_Command_Queue(D3D12_COMMAND_LIST_TYPE_COPY);
    auto command_list = command_queue->Get_Command_List();


    // Upload vertex buffer data.
    ComPtr<ID3D12Resource> temp_vertex_buffer;
    runtime->Get_DaVinci_instance()->Update_Buffer_Resource(
        command_list.Get(),
        &Cube_VertexBuffer,
        &temp_vertex_buffer,
        _countof(Cube_Vertexes),
        sizeof(VertexPosColor),
        Cube_Vertexes);

    // Create a VIEW for our Vertex Buffer
    Cube_VertexBufferView.BufferLocation = Cube_VertexBuffer->GetGPUVirtualAddress();
    Cube_VertexBufferView.SizeInBytes = sizeof(Cube_Vertexes);
    Cube_VertexBufferView.StrideInBytes = sizeof(VertexPosColor);

    // Upload up the index buffer data
    ComPtr<ID3D12Resource> temp_index_buffer;
    runtime->Get_DaVinci_instance()->Update_Buffer_Resource(
        command_list,
        &Cube_IndexBuffer,
        &temp_index_buffer,
        _countof(Cube_Indicies),
        sizeof(WORD),
        Cube_Indicies);

    // Create a VIEW for the vertex indeicies
    Cube_IndexBufferView.BufferLocation = Cube_IndexBuffer->GetGPUVirtualAddress();
    Cube_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    Cube_IndexBufferView.SizeInBytes = sizeof(Cube_Indicies);


    D3D12_DESCRIPTOR_HEAP_DESC depth_stencil_view_heap_desc = {};
    depth_stencil_view_heap_desc.NumDescriptors = 1;
    depth_stencil_view_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    depth_stencil_view_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(device->CreateDescriptorHeap(&depth_stencil_view_heap_desc, IID_PPV_ARGS(&m_DSVHeap))))
    {
        OhSHITc("How did we fail making a Depth Stencil View?");
        return false;
    }

    // Load up the vertex + pixel shaders
    ComPtr<ID3DBlob> vertex_shader_blob;
    ComPtr<ID3DBlob> pixel_shader_blob;
    if FAILED(
        D3DReadFileToBlob(L"VertexShader.cso", &vertex_shader_blob) ||
        D3DReadFileToBlob(L"PixelShader.cso", &pixel_shader_blob))
    {
        OhSHITc("FAILED TO READ SHADER FILES .CSO");
        return false;
    }

    // Create the vertex input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "COLOR",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
    };


    // Create a root signature
    D3D12_FEATURE_DATA_ROOT_SIGNATURE feature_data = {};
    feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &feature_data, sizeof(feature_data))))
    {
        feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS root_signature_flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;


    // A single 32-bit constant root parameter that is used by the vertex shader.
    CD3DX12_ROOT_PARAMETER1 root_parameters[1];
    root_parameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC root_signature_desc;
    root_signature_desc.Init_1_1(_countof(root_parameters), root_parameters, 0, nullptr, root_signature_flags);


    // Serialize the root signature.
    ComPtr<ID3DBlob> root_signature_blob;
    ComPtr<ID3DBlob> error_blob;

    D3DX12SerializeVersionedRootSignature(&root_signature_desc, feature_data.HighestVersion, &root_signature_blob,
                                          &error_blob);

    device->CreateRootSignature(0, root_signature_blob->GetBufferPointer(), root_signature_blob->GetBufferSize(),
                                IID_PPV_ARGS(&m_RootSignature));

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    pipelineStateStream.pRootSignature = m_RootSignature.Get();
    pipelineStateStream.InputLayout = {inputLayout, _countof(inputLayout)};
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertex_shader_blob.Get());
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixel_shader_blob.Get());
    pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof(PipelineStateStream), &pipelineStateStream
    };
    device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_PipelineState));

    auto fenceValue = command_queue->Execute_Command_List(command_list);
    command_queue->Wait_For_Fence_Value(fenceValue);

    // Resize/Create the depth buffer.
    ResizeDepthBuffer(runtime, runtime->Get_Window_Width(), runtime->Get_Window_Height());

    return true;
}


int main()
{
    INFOc("All good to go!")

    ENGINE::RUNTIME::ClientRuntime Runtime;
    Runtime.Create_Window("You got a 'Point'...", 1920, 1080);

    // Change Background
    Runtime.Get_DaVinci_instance()->Set_Clear_Colour(0.7f, 0.2f, 0.2f, 1.0f);

    // Vertex buffer for the cube.
    ComPtr<ID3D12Resource> m_VertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

    // Index buffer for the cube.
    ComPtr<ID3D12Resource> m_IndexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;


    
    // Update the model matrix.
    float angle = 0.0f;
    const XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);
    m_ModelMatrix = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));
    const XMVECTOR eyePosition = XMVectorSet(0, 0, -10, 1);
    const XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
    const XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
    m_ViewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);


    while (*Runtime.Is_Running() == true)
    {
        Runtime.Pre_tick();

        Runtime.Tick();

        Runtime.Post_tick();
        
        angle += 0.01f;
        float m_FoV = 45.0f;
        float aspectRatio = Runtime.Get_Window_Width() / static_cast<float>(Runtime.Get_Window_Height());
        m_ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FoV), aspectRatio, 0.1f, 1000.0f);
    }

    return 0;
}
