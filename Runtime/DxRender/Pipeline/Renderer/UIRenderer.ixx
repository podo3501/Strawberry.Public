module;

#include <wrl.h>
#include <DirectXMath.h>
#include "d3dx12.h"

export module Pipeline.Renderer:UI;

import std;
import :Config;
import :PipelineCache;
import :RootSignatureBuilder;
import :Utils;
import Core.Assert;
import Core.Math;
import Core.Utils;
import DxRender.Core;
import DxRender.Command;
import DxRender.Resource;
import DxRender.Helper;
import DxRender.Allocator;
import DxRender.Definition;
import Contract.Render.Definition;

using Microsoft::WRL::ComPtr;

struct UIDrawCB
{
    DirectX::XMFLOAT4X4 viewProj;
};

export class UIRenderer
{
public:
    ~UIRenderer() = default;

    UIRenderer(const UIRendererConfig& config, PipelineCache& pipelineCache) :
        m_config{ config },
        m_pipelineCache{ pipelineCache }
    {}

    bool Initialize(Device& device)
    {
        m_uiDrawCBAllocator.Initialize<UIDrawCB>(device, m_config.maxUI);

        if (!CreateRootSignature(device)) return false;
        m_pso = CreatePSO();
        if (!m_pso) return false;

        return true;
    }

    void ResetFrameResources(uint32_t slot)
    {
        m_uiDrawCBAllocator.Reset(slot); 
    }

    void BeginFrame(CommandList& cmd)
    {
        cmd.SetGraphicsRootSignature(m_rootSignature.Get());
        cmd.SetPipelineState(m_pso, PrimitiveTopologyType::Triangle);
    }

    void Draw(
        CommandList& cmd,
        MeshResource& mesh,
        const Core::Matrix& viewProj)
    {
        uint32_t resIndices[2] =
        {
            mesh.GetVertexHeapIndex(),
            mesh.GetIndexHeapIndex()
        };
        auto drawCBAddress = UploadDrawCB(viewProj);

        cmd->SetGraphicsRoot32BitConstants(Core::ToIndex(RootSlot::ResourceIndices), 2, resIndices, 0);
        cmd->SetGraphicsRootConstantBufferView(Core::ToIndex(RootSlot::DrawCB), drawCBAddress);

        cmd->DrawInstanced(mesh.GetIndexCount(), 1, 0, 0);
    }

private:
    enum class RootSlot : std::uint32_t
    {
        ResourceIndices = 0, // vb index, ib index, tex index
        DrawCB = 1
    };

    bool CreateRootSignature(Device& device)
    {
        RootSignatureBuilder builder;

        builder.Add32BitConstants(Core::ToIndex(RootSlot::ResourceIndices), 2);
        builder.AddCBV(Core::ToIndex(RootSlot::DrawCB));
        builder.AddLinearSampler(0);

        builder.AddFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        m_rootSignature = builder.Build(device);
        return m_rootSignature != nullptr;
    }

    ID3D12PipelineState* CreatePSO()
    {
        PipelineState pipelineState = PipelineLibrary::Get(
            RegistryShader::UI,
            RasterPreset::NoCull);

        return m_pipelineCache.GetOrCreate(
            pipelineState,
            m_rootSignature.Get(),
            [&](D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso)
            {
                pso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
                pso.DepthStencilState.DepthEnable = FALSE;
                pso.DepthStencilState.StencilEnable = FALSE;
                pso.DSVFormat = DXGI_FORMAT_UNKNOWN;

                SetPremultipliedAlphaBlend(pso.BlendState.RenderTarget[0]); // PMA로 설정
            });
    }

    D3D12_GPU_VIRTUAL_ADDRESS UploadDrawCB(const Core::Matrix& viewProj)
    {
        UIDrawCB drawCB{};
        DirectX::XMMATRIX xmViewProj = ToDXMatrix(viewProj);
        XMStoreFloat4x4(&drawCB.viewProj, DirectX::XMMatrixTranspose(xmViewProj));
        return m_uiDrawCBAllocator.AllocateConstant(drawCB);
    }

private:
    UIRendererConfig m_config;
    PipelineCache& m_pipelineCache;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ID3D12PipelineState* m_pso{ nullptr };

    FrameConstantAllocator m_uiDrawCBAllocator;
};