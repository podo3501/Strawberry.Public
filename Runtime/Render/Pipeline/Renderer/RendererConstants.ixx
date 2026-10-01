module;

#include <DirectXMath.h>

export module Pipeline.Renderer:Constants;

import std;
import Core.Bit;

export struct ObjectCB
{
    DirectX::XMFLOAT4X4 world;
};

export struct FrameCB
{
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 proj;
};

export struct MeshFrameCB
{
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 proj;
    DirectX::XMFLOAT4X4 lightViewProj;

    DirectX::XMFLOAT3 cameraPosition;
    float cameraPadding{ 0.f };

    DirectX::XMFLOAT3 lightDirection;
    float lightIntensity;

    DirectX::XMFLOAT3 lightColor;
    uint32_t shadowTextureIndex;

    uint32_t reflectionTextureIndex;
    uint32_t reflectionMipCount;
    float envPadding[2];
    DirectX::XMFLOAT4 irradianceSH[9];
};
static_assert(Core::IsSizeAligned<MeshFrameCB, 16>); // MeshFrameCB는 16의 배수여야함.