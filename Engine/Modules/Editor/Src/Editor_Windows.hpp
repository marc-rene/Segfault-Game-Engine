#pragma once

#include "Editor.hpp"

#include "../Runtime/ClientRuntime.hpp"

namespace ENGINE
{
    struct Editor::Windows
    {
        inline static void FPS_Counter()
        {
            bool bRunning = ENGINE::Runtime::ClientRuntime::IsRunning();
            ImGui::Begin("FPS Counter", NULL);

            ImGui::Text(" - - - FIXED TICK - - -");
            ImGui::Text("DeltaTime (ms): %f", ENGINE::Runtime::ClientRuntime::GetFixedTickDeltaTimeMiliSeconds());


            ImGui::Text("- - - RENDER TICK - - -");
            auto RDT = 1000.0f / ENGINE::Rendering::RenderMaster::GetInstance()->GetRenderTickDeltaTimeMicroSeconds();
            auto aRDT = 1000.0f / ENGINE::Rendering::RenderMaster::GetInstance()->GetRenderTickDeltaTimeMicroSeconds_Average();

            ImGui::Text("FPS: %f", 1000.0f / RDT);
            ImGui::Text("Average FPS: %f", 1000.0f / aRDT);
            ImGui::Text("DeltaTime (ms): %f", RDT);
            ImGui::Text("Average DeltaTime (ms): %f", aRDT);

            ImGui::End();
        }

        inline static void VideoSettings()
        {
            ImGui::Begin("FPS Counter", NULL);

        }
    };
}
