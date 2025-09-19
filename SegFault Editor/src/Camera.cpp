#include "Rendering.hpp"
#include "Level.hpp"

DirectX::XMMATRIX comp_Camera::GetProjectionMatrix(float p_fov, float p_aspectRatio, float p_nearPlane, float p_farPlane)
{
    return DirectX::XMMatrixPerspectiveFovRH(p_fov, p_aspectRatio, this->NearClipPlane, this->FarClipPlane);
}