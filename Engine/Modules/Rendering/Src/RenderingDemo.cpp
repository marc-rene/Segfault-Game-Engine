#include "Rendering.hpp"
#include "../Platform/Window.hpp"
#include "../Resources/EngineSettings.hpp"



long ENGINE::Rendering::RenderDemo::CreateSampleInputLayout(ID3DBlob* vsBlob)
{
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
    {
        { "POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    HRESULT hResult = GetDirectxDevice()->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &PrimaryInputLayout);
    assert(SUCCEEDED(hResult));
    vsBlob->Release();

    return hResult;
}

long ENGINE::Rendering::RenderDemo::CreateSampleTriangle(FLOAT Points[3][2], FLOAT Colours[3][4], UINT* numVerts_ptr, UINT* stride_ptr, UINT* offset_ptr)
{
    CompileVertexShaderFromFile("Examples\\Simple Triangle\\shaders.hlsl");
    CreateSampleInputLayout(VertexShaderBlob);
    
    CompilePixelShaderFromFile("Examples\\Simple Triangle\\shaders.hlsl");


    UINT numVerts;
    UINT stride;
    UINT offset;
    {
        float vertexData[] = { // x, y, r, g, b, a
            Points[0][0], Points[0][1], Colours[0][0], Colours[0][1], Colours[0][2], Colours[0][3],
            Points[1][0], Points[1][1], Colours[1][0], Colours[1][1], Colours[1][2], Colours[1][3],
            Points[2][0], Points[2][1], Colours[2][0], Colours[2][1], Colours[2][2], Colours[2][3],
        };
        stride = 6 * sizeof(float);
        numVerts = sizeof(vertexData) / stride;
        offset = 0;

        D3D11_BUFFER_DESC vertexBufferDesc = {};
        vertexBufferDesc.ByteWidth = sizeof(vertexData);
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexSubresourceData = { vertexData };

        HRESULT hResult = GetDirectxDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &PrimaryVertexBuffer);
        assert(SUCCEEDED(hResult));

        *numVerts_ptr = numVerts;
        *stride_ptr = stride;
        *offset_ptr = offset;
        return hResult;
    }
}

void ENGINE::Rendering::RenderDemo::DrawSampleTriangle(UINT* numVerts_ptr, UINT* stride_ptr, UINT* offset_ptr)
{
    RECT winRect;
    HWND hwnd = ENGINE::Platform::WindowManager::GetInstance()->GetMainWindowHWND();
    GetClientRect(hwnd, &winRect);
    D3D11_VIEWPORT viewport = 
    { 0.0f, 0.0f, (FLOAT)(winRect.right - winRect.left), (FLOAT)(winRect.bottom - winRect.top), 0.0f, 1.0f };
    device_context_ptr->RSSetViewports(1, &viewport);

    device_context_ptr->OMSetRenderTargets(1, &render_target_view_ptr, nullptr);

    device_context_ptr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_context_ptr->IASetInputLayout(PrimaryInputLayout);

    device_context_ptr->VSSetShader(PrimaryVertexShader, nullptr, 0);
    device_context_ptr->PSSetShader(PrimaryPixelShader, nullptr, 0);

    device_context_ptr->IASetVertexBuffers(0, 1, &PrimaryVertexBuffer, stride_ptr, offset_ptr);

    device_context_ptr->Draw(*numVerts_ptr, 0);
}

