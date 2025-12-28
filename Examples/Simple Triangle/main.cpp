#include <iostream>


#include "Apocalypse.hpp"



int main()
{
    ENGINE::Runtime::ClientRuntime clientRuntime;
    clientRuntime.RenderMasterInsatnce = reinterpret_cast<ENGINE::Rendering::RenderMaster*>(ENGINE::Rendering::RenderDemo::GetDemoInstance());
    ENGINE::Platform::WindowManager::GetInstance()->SetWindowTitle("Simple Triangle", true);

    FLOAT trianglePositions[3][2] = {
        {0.0f,  0.5f}, /*   / \     */
        {0.5f, -0.5f}, /*  /   \    */
        {-0.5f, -0.5f} /* /_____\   */  
    };

    FLOAT triangleColours[3][4] = {
        {0.f, 1.f, 0.f, 1.f},
        {1.f, 0.f, 0.f, 1.f},
        {0.f, 0.f, 1.f, 1.f}
    };
    UINT numVerts, stride, offset;

    clientRuntime.On_First_Ever_Frame();
    
    ENGINE::Rendering::RenderDemo::GetDemoInstance()->CreateSampleTriangle(trianglePositions, triangleColours, &numVerts, &stride, &offset);



    while (clientRuntime.IsRunning())
    {
        clientRuntime.On_FixedTick_Start();
        clientRuntime.On_Frame_Start();

        ENGINE::Rendering::RenderDemo::GetDemoInstance()->DrawSampleTriangle(&numVerts, &stride, &offset);

        clientRuntime.On_Frame_End();
        clientRuntime.On_FixedTick_End();
    }

    clientRuntime.Shutdown(); // Just incase something else changes clientRuntime.IsRunning()
    return 0;
}