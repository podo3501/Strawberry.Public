module;

#include <wrl.h>
#include <DirectXMath.h>
#include "d3dx12.h"

module DxRender.Pipeline.Renderer;

import :UI;
import :Utils;
import DxRender.Core;
import DxRender.Helper;
import Core.Assert;
import Core.Utils;
import Contract.Render.Backend.Types;

using Microsoft::WRL::ComPtr;

namespace
{
	enum class RootSlot : std::uint32_t
	{
		ResourceIndices = 0, // vb index, ib index, tex index
		DrawCB = 1
	};

	struct UIDrawCB
	{
		DirectX::XMFLOAT4X4 viewProj;
	};
}

UIRenderer::UIRenderer(const UIRendererConfig& config, PipelineCache& pipelineCache) :
    m_config{ config },
    m_pipelineCache{ pipelineCache }
{}

bool UIRenderer::Initialize(Device& device)
{
    m_uiDrawCBAllocator.Initialize<UIDrawCB>(device, m_config.maxUI);

    if (!CreateRootSignature(device)) return false;
    m_pso = CreatePSO();
    if (!m_pso) return false;

    return true;
}

void UIRenderer::ResetFrameResources(uint32_t slot)
{
    m_uiDrawCBAllocator.Reset(slot);
}

void UIRenderer::BeginFrame(CommandList& cmd)
{
    cmd.SetGraphicsRootSignature(m_rootSignature.Get());
    cmd.SetPipelineState(m_pso, PrimitiveTopologyType::Triangle);
}

void UIRenderer::Draw(
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

bool UIRenderer::CreateRootSignature(Device& device)
{
    RootSignatureBuilder builder;

    builder.Add32BitConstants(Core::ToIndex(RootSlot::ResourceIndices), 2);
    builder.AddCBV(Core::ToIndex(RootSlot::DrawCB));
    builder.AddLinearSampler(0);

    builder.AddFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    m_rootSignature = builder.Build(device);
    return m_rootSignature != nullptr;
}

ID3D12PipelineState* UIRenderer::CreatePSO()
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

D3D12_GPU_VIRTUAL_ADDRESS UIRenderer::UploadDrawCB(const Core::Matrix & viewProj)
{
    UIDrawCB drawCB{};
    DirectX::XMMATRIX xmViewProj = ToDXMatrix(viewProj);
    XMStoreFloat4x4(&drawCB.viewProj, DirectX::XMMatrixTranspose(xmViewProj));
    return m_uiDrawCBAllocator.AllocateConstant(drawCB);
}