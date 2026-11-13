module;

#include "d3dx12.h"

export module DxRender.Pipeline.Renderer:DebugSurface;

import std;
import :Config;
import :PipelineCache;
import Core.Math;
import DxRender.Core;
import DxRender.Allocator;
import DxRender.Command;
import DxRender.Definition;
import DxRender.Resource;
import Contract.Render.Shared.View;
import Contract.Render.Backend.View;

export class DebugSurfaceRenderer
{
public:
    ~DebugSurfaceRenderer() = default;
    DebugSurfaceRenderer() = delete;
    DebugSurfaceRenderer(const DebugSurfaceRendererConfig& config, PipelineCache& pipelineCache);

    bool Initialize(Device& device);
    void ResetFrameResources(std::uint32_t slot);
    // 프레임 상수 버퍼의 한도를 넘으면 false이고 아무것도 정하지 않는다. 부르는 쪽이 그 목록의 그리기를 건너뛴다.
    [[nodiscard]] bool PrepareDraw(CommandList& cmd, const CameraData& camera);
    // PSO를 얻지 못하면 false이고 아무것도 정하지 않는다. 부르는 쪽이 그 항목의 그리기를 건너뛴다.
    [[nodiscard]] bool BindPipeline(CommandList& cmd, const PipelineState& pipelineState);
    // 항목 상수 버퍼의 한도를 넘으면 false이고 그리지 않는다. 부르는 쪽이 나머지 항목을 건너뛴다.
    [[nodiscard]] bool Draw(CommandList& cmd, MeshResource& mesh, const Core::Matrix& world);

private:
    bool CreateRootSignature(Device& device);
    bool CreateDefaultPSOs();
    ID3D12PipelineState* CreatePSO(const PipelineState& pipelineState);
    ID3D12PipelineState* GetPipeline(const PipelineState& pipelineState);
    D3D12_GPU_VIRTUAL_ADDRESS UploadFrameCB(const CameraData& camera);
    D3D12_GPU_VIRTUAL_ADDRESS UploadObjectCB(const Core::Matrix& world);

    DebugSurfaceRendererConfig m_config;
    PipelineCache& m_pipelineCache;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

    FrameConstantAllocator m_objectCBAllocator;
    FrameConstantAllocator m_frameCBAllocator;
};