module;

#include <wrl.h>
#include <DirectXMath.h>
#include "d3dx12.h"

export module Pipeline.Renderer:Shadow;

import std;
import :Config;
import :PipelineCache;
import :RootSignatureBuilder;
import Core.Math;
import Core.Bit;
import Core.Utils;
import Runtime.Render.Core;
import Runtime.Render.Definition;
import Runtime.Render.Allocator;
import Runtime.Render.Command;
import Runtime.Render.Resource;
import Runtime.Render.Helper;
import Client.Render.Definition;
import Client.Render.View;

struct ShadowFrameCB
{
    DirectX::XMFLOAT4X4 lightViewProj;
};
static_assert(Core::IsSizeAligned<ShadowFrameCB, 16>);

struct ShadowObjectCB
{
    DirectX::XMFLOAT4X4 world;
};
static_assert(Core::IsSizeAligned<ShadowObjectCB, 16>);

enum class RootSlot : std::uint32_t
{
    MeshData = 0,
    FrameCB = 1,
    ObjectCB = 2
};

export class ShadowRenderer
{
public:
    ~ShadowRenderer() = default;
    ShadowRenderer() = delete;
    ShadowRenderer(const ShadowRendererConfig& config, PipelineCache& pipelineCache) :
        m_config{ config },
        m_pipelineCache{ pipelineCache }
    {}

    bool Initialize(Device& device)
    {
        m_objectCBAllocator.Initialize<ShadowObjectCB>(device, m_config.maxObjectCount);
        m_frameCBAllocator.Initialize<ShadowFrameCB>(device, m_config.frameCBCount);

        if (!CreateRootSignature(device))
            return false;

        m_shadowPSO = CreatePSO(PipelineLibrary::Get(RegistryShader::Shadow, RasterPreset::Default));
        if (!m_shadowPSO)
            return false;

        return true;
    }

    void ResetFrameResources(std::uint32_t slot)
    {
        m_objectCBAllocator.Reset(slot);
        m_frameCBAllocator.Reset(slot);
    }

    void PrepareDraw(CommandList& cmd, const DirectionalLightData& light)
    {
        auto frameCBAddress = UploadFrameCB(light);

        cmd.SetGraphicsRootSignature(m_rootSignature.Get());
        cmd.SetPipelineState(m_shadowPSO, PrimitiveTopologyType::Triangle); // 섀도우 맵은 항상 삼각형 리스트로 빌드업
        cmd->SetGraphicsRootConstantBufferView(Core::ToIndex(RootSlot::FrameCB), frameCBAddress);
    }

    void Draw(CommandList& cmd, MeshResource& mesh, const Core::Matrix& world)
    {
        auto objectCBAddress = UploadObjectCB(world);
        std::uint32_t meshData[2] = { mesh.GetVertexHeapIndex(), mesh.GetIndexHeapIndex() };

        cmd->SetGraphicsRoot32BitConstants(Core::ToIndex(RootSlot::MeshData), 2, meshData, 0);
        cmd->SetGraphicsRootConstantBufferView(Core::ToIndex(RootSlot::ObjectCB), objectCBAddress);
        cmd->DrawInstanced(mesh.GetIndexCount(), 1, 0, 0);
    }

private:
    bool CreateRootSignature(Device& device)
    {
        RootSignatureBuilder builder;

        builder.Add32BitConstants(Core::ToIndex(RootSlot::MeshData), 2);
        builder.AddCBV(Core::ToIndex(RootSlot::FrameCB));
        builder.AddCBV(Core::ToIndex(RootSlot::ObjectCB));

        builder.AddFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        m_rootSignature = builder.Build(device);
        return m_rootSignature != nullptr;
    }

    ID3D12PipelineState* CreatePSO(const PipelineState& pipelineState)
    {
        return m_pipelineCache.GetOrCreate(
            pipelineState,
            m_rootSignature.Get(),
            [&](D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso)
            {
                pso.NumRenderTargets = 0;
                pso.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;

                pso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
                pso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
                pso.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
                pso.DSVFormat = RenderFormat::ShadowMapFormat;

                pso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
                // 아티팩트 방지를 위한 하드웨어 뎁스 바이어스 설정 (수치는 상황에 따라 미세조정 필요)
                pso.RasterizerState.DepthBias = 3000;
                pso.RasterizerState.DepthBiasClamp = 0.0f;
                pso.RasterizerState.SlopeScaledDepthBias = 1.0f;

                pso.PS = CD3DX12_SHADER_BYTECODE(nullptr, 0);
            });
    }

    D3D12_GPU_VIRTUAL_ADDRESS UploadFrameCB(const DirectionalLightData& light)
    {
        ShadowFrameCB shadowFrame{};

        // 조명 데이터(light) 내부에 계산되어 저장되어 있을 Light View-Projection 행렬을 가져옴
        // 만약 Matrix 형식이 아니라면 프로젝트 변환 헬퍼(ToDXMatrix) 등을 거쳐 처리해줘
        DirectX::XMMATRIX lightVP = ToDXMatrix(light.viewProj);
        XMStoreFloat4x4(&shadowFrame.lightViewProj, DirectX::XMMatrixTranspose(lightVP));

        return m_frameCBAllocator.AllocateConstant(shadowFrame);
    }

    D3D12_GPU_VIRTUAL_ADDRESS UploadObjectCB(const Core::Matrix& world)
    {
        ShadowObjectCB obj{};
        DirectX::XMMATRIX xmWorld = ToDXMatrix(world);
        XMStoreFloat4x4(&obj.world, DirectX::XMMatrixTranspose(xmWorld));

        return m_objectCBAllocator.AllocateConstant(obj);
    }

    ShadowRendererConfig m_config;
    PipelineCache& m_pipelineCache;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    ID3D12PipelineState* m_shadowPSO{ nullptr };

    FrameConstantAllocator m_objectCBAllocator;
    FrameConstantAllocator m_frameCBAllocator;
};