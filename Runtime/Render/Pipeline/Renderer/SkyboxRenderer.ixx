module;

#include <wrl.h>
#include <DirectXMath.h>
#include "d3dx12.h"

export module Pipeline.Renderer:Skybox;

import std;
import :Config;
import :PipelineCache;
import :RootSignatureBuilder;
import Core.Utils;
import Core.Bit;
import Runtime.Render.Core;
import Runtime.Render.Command;
import Runtime.Render.Helper;
import Runtime.Render.Resource;
import Runtime.Render.Allocator;
import Runtime.Render.Definition;
import Client.Render.Definition;
import Client.Render.View;

using Microsoft::WRL::ComPtr;

struct SkyboxCB
{
    DirectX::XMFLOAT4X4 invViewProj; // translation 제거된 회전만 반영된 역행렬
    uint32_t skyboxTextureIndex;
    uint32_t padding[3];
};
static_assert(Core::IsSizeAligned<SkyboxCB, 16>);

enum class RootSlot : uint32_t
{
    FrameCB = 0,
};

export class SkyboxRenderer
{
public:
    ~SkyboxRenderer() = default;

    SkyboxRenderer(const SkyboxRendererConfig& config, PipelineCache& pipelineCache) :
        m_config{ config },
        m_pipelineCache{ pipelineCache }
    {}

    bool Initialize(Device& device)
    {
        m_cbAllocator.Initialize<SkyboxCB>(device, MaxViewCount);
        if (!CreateRootSignature(device)) return false;
        m_pso = CreatePSO();
        return m_pso != nullptr;
    }

    void ResetFrameResources(uint32_t slot)
    {
        m_cbAllocator.Reset(slot);
    }

    void Draw(CommandList& cmd, const CameraData& camera, TextureCubeResource& skybox)
    {
        // translation 제거 - 카메라 위치를 원점으로 고정해서 스카이박스가 "무한히 먼 배경"처럼 보이게 함
        DirectX::XMMATRIX view = ToDXMatrix(camera.view);
        view.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f); // 4행(translation 성분) 제거
        DirectX::XMMATRIX proj = ToDXMatrix(camera.proj);
        DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiply(view, proj);
        DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(nullptr, viewProj);

        SkyboxCB cb{};
        XMStoreFloat4x4(&cb.invViewProj, DirectX::XMMatrixTranspose(invViewProj));
        cb.skyboxTextureIndex = skybox.GetHeapIndex();

        auto cbAddress = m_cbAllocator.AllocateConstant(cb);

        cmd.SetGraphicsRootSignature(m_rootSignature.Get());
        cmd.SetPipelineState(m_pso, PrimitiveTopologyType::Triangle);
        cmd->SetGraphicsRootConstantBufferView(Core::ToIndex(RootSlot::FrameCB), cbAddress);
        cmd->DrawInstanced(3, 1, 0, 0); // 풀스크린 트라이앵글 - 버텍스/인덱스 버퍼 없음
    }

private:
    bool CreateRootSignature(Device& device)
    {
        RootSignatureBuilder builder;
        builder.AddCBV(Core::ToIndex(RootSlot::FrameCB)); // SkyboxCB를 여기에 바인딩 (SurfaceRenderer의 ObjectCB/MaterialCB 슬롯은 안 씀)
        builder.AddFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT); // 버텍스 입력 자체를 안 쓰지만 플래그는 통일성 위해 유지 (불필요하면 제거 가능)
        builder.AddLinearSampler(0);

        m_rootSignature = builder.Build(device);
        return m_rootSignature != nullptr;
    }

    ID3D12PipelineState* CreatePSO()
    {
        PipelineState pipelineState = PipelineLibrary::Get(
            RegistryShader::Skybox,
            RasterPreset::Default,
            PrimitiveTopologyType::Triangle);

        return m_pipelineCache.GetOrCreate(
            pipelineState,
            m_rootSignature.Get(),
            [&](D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso)
            {
                pso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
                pso.DepthStencilState.DepthEnable = TRUE;
                pso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // write 안 함 - 항상 배경이라 다른 오브젝트가 덮어써야 함
                pso.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // z=1로 그리므로 LESS_EQUAL 필요
                pso.DSVFormat = RenderFormat::DepthFormat;
                pso.InputLayout = { nullptr, 0 }; // 버텍스 입력 레이아웃 없음 - SV_VertexID만 사용
            });
    }

private:
    SkyboxRendererConfig m_config;
    PipelineCache& m_pipelineCache;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ID3D12PipelineState* m_pso{ nullptr };

    FrameConstantAllocator m_cbAllocator;
};