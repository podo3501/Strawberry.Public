module;

#include <wrl.h>
#include <DirectXMath.h>
#include "d3dx12.h"

export module Pipeline.Renderer:DebugSurface;

import std;
import :Config;
import :Constants;
import :PipelineCache;
import :RootSignatureBuilder;
import Core.Math;
import Core.Bit;
import Core.Utils;
import Runtime.Render.Core;
import Runtime.Render.Allocator;
import Runtime.Render.Command;
import Runtime.Render.Definition;
import Runtime.Render.Resource;
import Runtime.Render.Helper;
import Client.Render.Definition;
import Client.Render.View;

//struct FrameCB
//{
//    DirectX::XMFLOAT4X4 view;
//    DirectX::XMFLOAT4X4 proj;
//};
//static_assert(Core::IsSizeAligned<FrameCB, 16>);

//struct ObjectCB
//{
//    DirectX::XMFLOAT4X4 world;
//};
//static_assert(Core::IsSizeAligned<ObjectCB, 16>);

export class DebugSurfaceRenderer
{
public:
    ~DebugSurfaceRenderer() = default;
    DebugSurfaceRenderer() = delete;
    DebugSurfaceRenderer(const DebugSurfaceRendererConfig& config, PipelineCache& pipelineCache)
        : m_config{ config }
        , m_pipelineCache{ pipelineCache }
    {
    }

    bool Initialize(Device& device)
    {
        m_objectCBAllocator.Initialize<ObjectCB>(device, m_config.maxObjectCount);
        m_frameCBAllocator.Initialize<FrameCB>(device, MaxViewCount);

        if (!CreateRootSignature(device)) return false;
        if (!CreateDefaultPSOs()) return false;

        return true;
    }

    void ResetFrameResources(std::uint32_t slot)
    {
        m_objectCBAllocator.Reset(slot);
        m_frameCBAllocator.Reset(slot);
    }

    void PrepareDraw(CommandList& cmd, const CameraData& camera)
    {
        auto frameCBAddress = UploadFrameCB(camera);

        cmd.SetGraphicsRootSignature(m_rootSignature.Get());
        cmd->SetGraphicsRootConstantBufferView(Core::ToIndex(RootSlot::FrameCB), frameCBAddress);
    }

    void BindPipeline(CommandList& cmd, const PipelineState& pipelineState)
    {
        cmd.SetPipelineState(GetPipeline(pipelineState), pipelineState.topologyType);
    }

    void Draw(CommandList& cmd, MeshResource& mesh, const Core::Matrix& world)
    {
        auto objectCBAddress = UploadObjectCB(world);
        std::uint32_t vbIndex = mesh.GetVertexHeapIndex();

        cmd->SetGraphicsRoot32BitConstants(Core::ToIndex(RootSlot::VertexIndex), 1, &vbIndex, 0);
        cmd->SetGraphicsRootConstantBufferView(Core::ToIndex(RootSlot::ObjectCB), objectCBAddress);

        cmd->DrawInstanced(mesh.GetVertexCount(), 1, 0, 0);
    }

private:
    enum class RootSlot : std::uint32_t
    {
        VertexIndex = 0,
        FrameCB = 1,
        ObjectCB = 2
    };

    bool CreateRootSignature(Device& device)
    {
        RootSignatureBuilder builder;

        builder.Add32BitConstants(Core::ToIndex(RootSlot::VertexIndex), 1);
        builder.AddCBV(Core::ToIndex(RootSlot::FrameCB));
        builder.AddCBV(Core::ToIndex(RootSlot::ObjectCB));

        builder.AddFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        m_rootSignature = builder.Build(device);
        return m_rootSignature != nullptr;
    }

    bool CreateDefaultPSOs()
    {
        return CreatePSO(PipelineLibrary::Get(RegistryShader::Grid, RasterPreset::Default, PrimitiveTopologyType::Line)) != nullptr;
    }

    ID3D12PipelineState* GetPipeline(const PipelineState& pipelineState)
    {
        auto* pipeline = m_pipelineCache.Find(pipelineState);
        if (pipeline)
            return pipeline;

        return CreatePSO(pipelineState);
    }

    ID3D12PipelineState* CreatePSO(const PipelineState& pipelineState)
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

    D3D12_GPU_VIRTUAL_ADDRESS UploadFrameCB(const CameraData& camera)
    {
        FrameCB frame{};
        DirectX::XMMATRIX view = ToDXMatrix(camera.view);
        DirectX::XMMATRIX proj = ToDXMatrix(camera.proj);

        // GPU용으로 transpose해서 저장
        XMStoreFloat4x4(&frame.view, DirectX::XMMatrixTranspose(view));
        XMStoreFloat4x4(&frame.proj, DirectX::XMMatrixTranspose(proj));

        return m_frameCBAllocator.AllocateConstant(frame);
    }

    D3D12_GPU_VIRTUAL_ADDRESS UploadObjectCB(const Core::Matrix& world)
    {
        ObjectCB obj{};

        DirectX::XMMATRIX xmWorld = ToDXMatrix(world);
        XMStoreFloat4x4(&obj.world, DirectX::XMMatrixTranspose(xmWorld));

        return m_objectCBAllocator.AllocateConstant(obj);
    }

    DebugSurfaceRendererConfig m_config;
    PipelineCache& m_pipelineCache;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

    FrameConstantAllocator m_objectCBAllocator;
    FrameConstantAllocator m_frameCBAllocator;
};