module;

#include "d3dx12.h"

export module Pipeline.Renderer:DebugSurface;

import std;
import :Config;
import :PipelineCache;
import Core.Math;
import DxRender.Core;
import DxRender.Allocator;
import DxRender.Command;
import DxRender.Definition;
import DxRender.Resource;
import Contract.Render.Backend.Types;
import Contract.Render.Backend.View;

export class DebugSurfaceRenderer
{
public:
    ~DebugSurfaceRenderer() = default;
    DebugSurfaceRenderer() = delete;
    DebugSurfaceRenderer(const DebugSurfaceRendererConfig& config, PipelineCache& pipelineCache);

    bool Initialize(Device& device);
    void ResetFrameResources(std::uint32_t slot);
    void PrepareDraw(CommandList& cmd, const CameraData& camera);
    void BindPipeline(CommandList& cmd, const PipelineState& pipelineState);
    void Draw(CommandList& cmd, MeshResource& mesh, const Core::Matrix& world);

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