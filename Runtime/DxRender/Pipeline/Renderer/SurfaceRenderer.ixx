module;

#include <wrl.h>
#include <DirectXMath.h>
#include "d3dx12.h"

export module Pipeline.Renderer:Surface;

import std;
import :Config;
import :PipelineCache;
import :Constants;
import :RootSignatureBuilder;
import Core.Math;
import Core.Utils;
import Core.Bit;
import DxRender.Core;
import DxRender.Command;
import DxRender.Helper;
import DxRender.Resource;
import DxRender.Allocator;
import DxRender.Definition;
import Contract.Render.Definition;
import Contract.Render.View;

using Microsoft::WRL::ComPtr;

//xxxStrength: 0에서 1 사이.
//xxxScale: 0에서 무한대. 원본값을 스케일하는 것.
//xxxIntensity: 0에서 무한대. 대신 원본값.

struct PbrMaterialCB
{
    uint32_t albedoTextureIndex;
    uint32_t normalTextureIndex;
    uint32_t armTextureIndex;
    float normalScale;
    float roughnessScale;
    float metallicScale;
    float aoStrength;
    float padding{ 0.f };
};

struct PhongMaterialCB
{
    uint32_t albedoTextureIndex;
    uint32_t normalTextureIndex;
    uint32_t dummyTextureIndex{ 0 };
    float normalScale;
    float ambientScale;
    float specularScale;
    float shininess; // 하이라이트 지수: 보통 4.0 ~ 256.0
    float padding{ 0.f };
};

struct MaterialConstantBuffer
{
    uint32_t textureIndices[3]; // 텍스처 인덱스
    float    param[5];
};
static_assert(Core::IsSizeAligned<MaterialConstantBuffer, 16>);

static_assert(sizeof(PbrMaterialCB) == sizeof(MaterialConstantBuffer));
static_assert(sizeof(PhongMaterialCB) == sizeof(MaterialConstantBuffer));

export class SurfaceRenderer
{
public:
    ~SurfaceRenderer() = default;
    SurfaceRenderer() = delete;
    SurfaceRenderer(const SurfaceRendererConfig& config, PipelineCache& pipelineCache) :
        m_config{ config },
        m_pipelineCache{ pipelineCache }
    {
    }

    bool Initialize(Device& device)
    {
        m_objectCBAllocator.Initialize<ObjectCB>(device, m_config.maxObjectCount);
        m_materialCBAllocator.Initialize<MaterialConstantBuffer>(device, m_config.maxObjectCount);
        m_frameCBAllocator.Initialize<MeshFrameCB>(device, MaxViewCount);

        if (!CreateRootSignature(device)) return false;
        if (!CreateDefaultPSOs()) return false;

        return true;
    }

    void ResetFrameResources(uint32_t slot)
    {
        m_objectCBAllocator.Reset(slot);
        m_materialCBAllocator.Reset(slot);
        m_frameCBAllocator.Reset(slot);
    }

    void PrepareDraw(
        CommandList& cmd,
        const DirectionalLightData& light,
        const CameraData& camera,
        uint32_t shadowSRVIndex,
        const EnvironmentResource* envRes)
    {
        auto frameCBAddress = UploadFrameCB(
            light,
            camera,
            shadowSRVIndex,
            envRes);

        cmd.SetGraphicsRootSignature(m_rootSignature.Get());
        cmd->SetGraphicsRootConstantBufferView(Core::ToIndex(RootSlot::FrameCB), frameCBAddress);
    }

    void BindPipeline(CommandList& cmd, const PipelineState& pipelineState)
    {
        auto* pso = GetPipeline(pipelineState);
        cmd.SetPipelineState(pso, pipelineState.topologyType);
    }

    void Draw(
        CommandList& cmd,
        MeshResource& mesh,
        MaterialResource& material,
        const Core::Matrix& world)
    {
        auto objectCBAddress = UploadObjectCB(world);
        auto materialCBAddress = UploadMaterialCB(material);
        uint32_t meshIndices[2] = { mesh.GetVertexHeapIndex(), mesh.GetIndexHeapIndex() };

        cmd->SetGraphicsRoot32BitConstants(Core::ToIndex(RootSlot::MeshData), 2, meshIndices, 0);
        cmd->SetGraphicsRootConstantBufferView(Core::ToIndex(RootSlot::ObjectCB), objectCBAddress);
        cmd->SetGraphicsRootConstantBufferView(Core::ToIndex(RootSlot::MaterialCB), materialCBAddress);

        cmd->DrawInstanced(mesh.GetIndexCount(), 1, 0, 0);
    }

private:
    enum class RootSlot : uint32_t
    {
        MeshData = 0,
        FrameCB = 1,
        ObjectCB = 2,
        MaterialCB = 3
    };

    bool CreateRootSignature(Device& device)
    {
        RootSignatureBuilder builder;

        builder.Add32BitConstants(Core::ToIndex(RootSlot::MeshData), 2);
        builder.AddCBV(Core::ToIndex(RootSlot::FrameCB));
        builder.AddCBV(Core::ToIndex(RootSlot::ObjectCB));
        builder.AddCBV(Core::ToIndex(RootSlot::MaterialCB));

        builder.AddFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        builder.AddLinearSampler(0);
        builder.AddComparisonSampler(1);

        m_rootSignature = builder.Build(device);
        return m_rootSignature != nullptr;
    }

    bool CreateDefaultPSOs()
    {
        if (CreatePSO(PipelineLibrary::Get(RegistryShader::Phong, RasterPreset::Default)) == nullptr) return false;
        if (CreatePSO(PipelineLibrary::Get(RegistryShader::Phong, RasterPreset::NoCull)) == nullptr) return false;
        if (CreatePSO(PipelineLibrary::Get(RegistryShader::Phong, RasterPreset::Wireframe)) == nullptr) return false;
        if (CreatePSO(PipelineLibrary::Get(RegistryShader::Phong, RasterPreset::WireframeNoCull)) == nullptr) return false;

        if (CreatePSO(PipelineLibrary::Get(RegistryShader::PBR, RasterPreset::Default)) == nullptr) return false;

        return true;
    }

    ID3D12PipelineState* CreatePSO(const PipelineState& pipelineState)
    {
        return m_pipelineCache.GetOrCreate(
            pipelineState,
            m_rootSignature.Get(),
            [&](D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso)
            {
                pso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
                pso.DepthStencilState.DepthEnable = TRUE;
                pso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
                pso.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
                pso.DSVFormat = RenderFormat::DepthFormat;
            });
    }

    ID3D12PipelineState* GetPipeline(const PipelineState& pipelineState)
    {
        auto* pipeline = m_pipelineCache.Find(pipelineState);
        if (pipeline)
            return pipeline;

        return CreatePSO(pipelineState);
    }

    D3D12_GPU_VIRTUAL_ADDRESS UploadFrameCB(
        const DirectionalLightData& light,
        const CameraData& camera,
        uint32_t shadowSRVIndex,
        const EnvironmentResource* envRes)
    {
        MeshFrameCB meshFrame{};

        DirectX::XMMATRIX view = ToDXMatrix(camera.view);
        DirectX::XMMATRIX proj = ToDXMatrix(camera.proj);
        DirectX::XMMATRIX lightVP = ToDXMatrix(light.viewProj);

        XMStoreFloat4x4(
            &meshFrame.view,
            DirectX::XMMatrixTranspose(view));

        XMStoreFloat4x4(
            &meshFrame.proj,
            DirectX::XMMatrixTranspose(proj));

        XMStoreFloat4x4(
            &meshFrame.lightViewProj,
            DirectX::XMMatrixTranspose(lightVP));

        meshFrame.cameraPosition = ToXMFLOAT3(camera.position);
        meshFrame.lightDirection = ToXMFLOAT3(light.direction);
        meshFrame.lightIntensity = light.intensity;
        meshFrame.lightColor = ToXMFLOAT3(light.color);
        meshFrame.shadowTextureIndex = shadowSRVIndex;

        if (envRes && envRes->IsReady())
        {
            meshFrame.reflectionTextureIndex =
                envRes->GetReflection()->GetHeapIndex();

            meshFrame.reflectionMipCount =
                envRes->GetReflection()->GetMipCount();

            const auto& sh = envRes->GetIrradianceSH();

            for (int i = 0; i < 9; ++i)
                meshFrame.irradianceSH[i] =
            { sh[i].x, sh[i].y, sh[i].z, 0.0f };
        }
        else
        {
            meshFrame.reflectionTextureIndex = UINT_MAX;
            meshFrame.reflectionMipCount = 0;
        }

        return m_frameCBAllocator.AllocateConstant(meshFrame);
    }

    D3D12_GPU_VIRTUAL_ADDRESS UploadObjectCB(const Core::Matrix& world)
    {
        ObjectCB obj{};

        DirectX::XMMATRIX xmWorld = ToDXMatrix(world);
        XMStoreFloat4x4(&obj.world, DirectX::XMMatrixTranspose(xmWorld));

        return m_objectCBAllocator.AllocateConstant(obj);
    }

    D3D12_GPU_VIRTUAL_ADDRESS UploadMaterialCB(MaterialResource& material)
    {
        MaterialConstantBuffer gpuCB{};

        switch (material.GetType())
        {
        case MaterialType::Phong:
        {
            auto& phongMat = static_cast<PhongMaterialResource&>(material);
            const PhongSurface& surface = phongMat.GetSurface();

            PhongMaterialCB cb{};
            cb.albedoTextureIndex = phongMat.GetAlbedo().GetHeapIndex();
            cb.normalTextureIndex = phongMat.GetNormal().GetHeapIndex();
            cb.dummyTextureIndex = 0;

            cb.normalScale = surface.normalScale;
            cb.ambientScale = surface.ambientScale;
            cb.specularScale = surface.specularScale;
            cb.shininess = surface.shininess;

            std::memcpy(&gpuCB, &cb, sizeof(MaterialConstantBuffer));
            break;
        }
        case MaterialType::PBR:
        {
            auto& pbrMat = static_cast<PbrMaterialResource&>(material);
            const PbrSurface& surface = pbrMat.GetSurface();

            PbrMaterialCB cb{};
            cb.albedoTextureIndex = pbrMat.GetAlbedo().GetHeapIndex();
            cb.normalTextureIndex = pbrMat.GetNormal().GetHeapIndex();
            cb.armTextureIndex = pbrMat.GetArm().GetHeapIndex();

            cb.normalScale = surface.normalScale;
            cb.roughnessScale = surface.roughnessScale;
            cb.metallicScale = surface.metallicScale;
            cb.aoStrength = surface.aoStrength;

            std::memcpy(&gpuCB, &cb, sizeof(MaterialConstantBuffer));
            break;
        }
        }

        return m_materialCBAllocator.AllocateConstant(gpuCB);
    }

private:
    SurfaceRendererConfig m_config;
    PipelineCache& m_pipelineCache;
    ComPtr<ID3D12RootSignature> m_rootSignature;

    FrameConstantAllocator m_objectCBAllocator;
    FrameConstantAllocator m_materialCBAllocator;
    FrameConstantAllocator m_frameCBAllocator;

    D3D12_GPU_VIRTUAL_ADDRESS m_frameCBAddress{};
};