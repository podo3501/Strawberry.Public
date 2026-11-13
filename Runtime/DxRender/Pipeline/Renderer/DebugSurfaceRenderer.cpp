module;

#include <wrl.h>
#include <DirectXMath.h>
#include "d3dx12.h"

module DxRender.Pipeline.Renderer;

import :DebugSurface;
import :Constants;
import DxRender.Core;
import DxRender.Allocator;
import DxRender.Helper;
import DxRender.Definition;
import Core.Bit;
import Core.Utils;
import Contract.Render.Backend;

namespace
{
    enum class RootSlot : std::uint32_t
    {
        VertexIndex = 0,
        FrameCB = 1,
        ObjectCB = 2
    };
}

DebugSurfaceRenderer::DebugSurfaceRenderer(const DebugSurfaceRendererConfig & config, PipelineCache & pipelineCache)
    : m_config{ config }
    , m_pipelineCache{ pipelineCache }
{}

bool DebugSurfaceRenderer::Initialize(Device& device)
{
    if (!m_objectCBAllocator.Initialize<ObjectCB>(device, m_config.maxObjectCount)) return false;
    if (!m_frameCBAllocator.Initialize<FrameCB>(device, MaxViewCount)) return false;

    if (!CreateRootSignature(device)) return false;
    if (!CreateDefaultPSOs()) return false;

    return true;
}

void DebugSurfaceRenderer::ResetFrameResources(std::uint32_t slot)
{
    m_objectCBAllocator.Reset(slot);
    m_frameCBAllocator.Reset(slot);
}

bool DebugSurfaceRenderer::PrepareDraw(CommandList& cmd, const CameraData& camera)
{
    auto frameCBAddress = UploadFrameCB(camera);
    if (frameCBAddress == FrameConstantAllocator::InvalidAddress)
        return false;

    cmd.SetGraphicsRootSignature(m_rootSignature.Get());
    cmd->SetGraphicsRootConstantBufferView(Core::ToIndex(RootSlot::FrameCB), frameCBAddress);
    return true;
}

bool DebugSurfaceRenderer::BindPipeline(CommandList& cmd, const PipelineState& pipelineState)
{
    auto* pso = GetPipeline(pipelineState);
    if (!pso)
        return false; // 원인은 PipelineCache가 이미 Error로 남겼다

    cmd.SetPipelineState(pso, pipelineState.topologyType);
    return true;
}

bool DebugSurfaceRenderer::Draw(CommandList& cmd, MeshResource& mesh, const Core::Matrix& world)
{
    auto objectCBAddress = UploadObjectCB(world);
    if (objectCBAddress == FrameConstantAllocator::InvalidAddress)
        return false;

    std::uint32_t vbIndex = mesh.GetVertexHeapIndex();

    cmd->SetGraphicsRoot32BitConstants(Core::ToIndex(RootSlot::VertexIndex), 1, &vbIndex, 0);
    cmd->SetGraphicsRootConstantBufferView(Core::ToIndex(RootSlot::ObjectCB), objectCBAddress);

    cmd->DrawInstanced(mesh.GetVertexCount(), 1, 0, 0);
    return true;
}

bool DebugSurfaceRenderer::CreateRootSignature(Device& device)
{
    RootSignatureBuilder builder;

    builder.Add32BitConstants(Core::ToIndex(RootSlot::VertexIndex), 1);
    builder.AddCBV(Core::ToIndex(RootSlot::FrameCB));
    builder.AddCBV(Core::ToIndex(RootSlot::ObjectCB));

    builder.AddFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    m_rootSignature = builder.Build(device);
    return m_rootSignature != nullptr;
}

bool DebugSurfaceRenderer::CreateDefaultPSOs()
{
    return CreatePSO(PipelineLibrary::Get(RegistryShader::Grid, RasterPreset::Default, PrimitiveTopologyType::Line)) != nullptr;
}

ID3D12PipelineState* DebugSurfaceRenderer::GetPipeline(const PipelineState& pipelineState)
{
    auto* pipeline = m_pipelineCache.Find(pipelineState);
    if (pipeline)
        return pipeline;

    return CreatePSO(pipelineState);
}

ID3D12PipelineState* DebugSurfaceRenderer::CreatePSO(const PipelineState& pipelineState)
{
    return m_pipelineCache.GetOrCreate(
        pipelineState,
        m_rootSignature.Get(),
        [&](D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso)
        {
            pso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
            pso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
            pso.DSVFormat = RenderFormat::DepthFormat;
        });
}

D3D12_GPU_VIRTUAL_ADDRESS DebugSurfaceRenderer::UploadFrameCB(const CameraData& camera)
{
    FrameCB frame{};
    DirectX::XMMATRIX view = ToDXMatrix(camera.view);
    DirectX::XMMATRIX proj = ToDXMatrix(camera.proj);

    // GPU용으로 transpose해서 저장
    XMStoreFloat4x4(&frame.view, DirectX::XMMatrixTranspose(view));
    XMStoreFloat4x4(&frame.proj, DirectX::XMMatrixTranspose(proj));

    return m_frameCBAllocator.AllocateConstant(frame);
}

D3D12_GPU_VIRTUAL_ADDRESS DebugSurfaceRenderer::UploadObjectCB(const Core::Matrix& world)
{
    ObjectCB obj{};

    DirectX::XMMATRIX xmWorld = ToDXMatrix(world);
    XMStoreFloat4x4(&obj.world, DirectX::XMMatrixTranspose(xmWorld));

    return m_objectCBAllocator.AllocateConstant(obj);
}