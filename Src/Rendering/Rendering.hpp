#pragma once

#include "../Core/TypeDefinitions.hpp"
#include "../Core/Log.hpp"

#define DIRECTX11 0
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#if DIRECTX11
#include <d3d11_1.h>       
#else
#include <d3d12.h>       
#endif

#include <dxgi1_6.h>        // DirectX driver interface
#include <d3dcompiler.h>    // shader compiler
#pragma comment(lib, "d3dcompiler.lib")

#include <iostream>
#include <chrono>
#include <mutex>





namespace ENGINE::Rendering
{

    class RenderMaster
    {
    public:
        bool Initialise();

        void ClearRenderTarget()
        {
            static FLOAT ClearColour[4] = { 0.85f, 0.09f, 0.11f, 1.0f };
            device_context_ptr->ClearRenderTargetView(render_target_view_ptr, ClearColour);
        }

        void ShowCompletedFrame();

        void On_RenderStart()
        {
            DT_Mutex.lock();
            RenderTickStartTimestamp = std::chrono::high_resolution_clock::now();
            DT_Mutex.unlock();

            
            return;
        };

        void On_RenderEnd()
        {
            ShowCompletedFrame();

            DT_Mutex.lock();

            // STOP THE CLOCK 
            auto FinishedTimestamp = std::chrono::high_resolution_clock::now();
            LastRenderTickDeltaTimeMicroSeconds = std::chrono::duration_cast<std::chrono::microseconds>(FinishedTimestamp - RenderTickStartTimestamp);

            if (LastRenderTickDeltaTimeMicroSeconds.count() < 1) // We cant have 0 microseconds
                LastRenderTickDeltaTimeMicroSeconds = LastRenderTickDeltaTimeMicroSeconds.zero();

            AverageRenderTickDT_DataSet[FenceValue % 32] = LastRenderTickDeltaTimeMicroSeconds.count();

            

            FenceValue++;
            DT_Mutex.unlock();
            
            return;
        }

        long ResizeSwapChainBuffer();

        long CompileVertexShaderFromFile(std::string ShaderFileName);
        long DefineInputLayout(ID3DBlob* vsBlob);
        long CompilePixelShaderFromFile(std::string ShaderFileName);


#if DIRECTX11
        ID3D11Device1* GetDirectxDevice() const
        {
            return device_ptr;
        }
#else
        ID3D12Device1* GetDirectxDevice() const 
        {
            return device_ptr;
        }
#endif

        inline static RenderMaster* GetInstance()
        {
            if (Self_ptr == nullptr) {
                creation_mtx.lock();

                if (Self_ptr == nullptr)
                    Self_ptr = new RenderMaster();
                creation_mtx.unlock();
            }
            
            return Self_ptr;

        }

        inline static const long long GetRenderTickDeltaTimeMicroSeconds()
        {
            return LastRenderTickDeltaTimeMicroSeconds.count();
        }
        inline static const float GetRenderTickDeltaTimeMiliSeconds()
        {
            return LastRenderTickDeltaTimeMicroSeconds.count() / 1000.0f;
        }
        inline static const double GetRenderTickDeltaTimeMiliSeconds_double()
        {
            return LastRenderTickDeltaTimeMicroSeconds.count() / 1000.0;
        }

        inline static long long GetRenderTickDeltaTimeMicroSeconds_Average()
        {
            long long temp;
            temp = 0;
            for (int i = 0; i < 32; i++)
            {
                temp += AverageRenderTickDT_DataSet[i];
            }
            return (temp / 32);
        }


#if DIRECTX11
        ID3D11Device1* device_ptr;
        ID3D11DeviceContext1* device_context_ptr;
        IDXGISwapChain1* swap_chain_ptr;
        ID3D11RenderTargetView* render_target_view_ptr;
        DXGI_SWAP_CHAIN_DESC    swap_chain_descr;
        ID3D11VertexShader* PrimaryVertexShader;
        ID3D11Buffer* PrimaryVertexBuffer;
        ID3D11PixelShader* PrimaryPixelShader;
        ID3D11InputLayout* PrimaryInputLayout;
        ID3DBlob* VertexShaderBlob;
#else
        ID3D12Device* device_ptr;
        IDXGISwapChain* swap_chain_ptr;
        ID3D11RenderTargetView* render_target_view_ptr;
        DXGI_SWAP_CHAIN_DESC    swap_chain_descr;
        ID3D11VertexShader* PrimaryVertexShader;
        ID3D11Buffer* PrimaryVertexBuffer;
        ID3D11PixelShader* PrimaryPixelShader;
        ID3D11InputLayout* PrimaryInputLayout;
        ID3DBlob* VertexShaderBlob;
    
#endif
        RenderMaster() {
            device_ptr = NULL;
            device_context_ptr = NULL;
            swap_chain_ptr = NULL;
            render_target_view_ptr = NULL;
            swap_chain_descr = {};
            PrimaryVertexShader = NULL;
            PrimaryVertexBuffer = NULL;
            PrimaryPixelShader = NULL;
            PrimaryInputLayout = NULL;
            VertexShaderBlob = NULL;
        };

    
    
        inline static std::chrono::steady_clock::time_point RenderTickStartTimestamp;
        inline static std::chrono::microseconds LastRenderTickDeltaTimeMicroSeconds;
        inline static long long AverageRenderTickDT_DataSet[32] = {};
        inline static std::mutex DT_Mutex;
        inline static unsigned long long FenceValue = 0;
        inline static std::mutex creation_mtx;
        inline static RenderMaster* Self_ptr;
    };

    class RenderDemo : private RenderMaster
    {
    public:
        long CreateSampleInputLayout(ID3DBlob* vsBlob);

        long CreateSampleTriangle(FLOAT Points[3][2], FLOAT Colours[3][4], UINT* numVerts_ptr, UINT* stride_ptr, UINT* offset_ptr);

        void DrawSampleTriangle(UINT* numVerts_ptr, UINT* stride_ptr, UINT* offset_ptr);

        inline static RenderDemo* GetDemoInstance()
        {
            if (Demo_ptr == nullptr) {
                creation_mtx.lock();

                if (Demo_ptr == nullptr)
                    Demo_ptr = new RenderDemo();
                creation_mtx.unlock();
            }

            return Demo_ptr;
        }

        inline static RenderDemo* Demo_ptr;

        RenderDemo() : RenderMaster()
        {
        };


    };
};

