module;

#include "d3dx12.h"

export module Pipeline.Renderer:Surface;

import std;
import :Config;
import :PipelineCache;
import Core.Math;
import DxRender.Core;
import DxRender.Command;
import DxRender.Resource;
import DxRender.Allocator;
import DxRender.Definition;
import Contract.Render.View;

using Microsoft::WRL::ComPtr;

export class SurfaceRenderer
{
public:
    ~SurfaceRenderer() = default;
    SurfaceRenderer() = delete;
    SurfaceRenderer(const SurfaceRendererConfig& config, PipelineCache& pipelineCache);

    bool Initialize(Device& device);
    void ResetFrameResources(uint32_t slot);

    void PrepareDraw(
        CommandList& cmd,
        const DirectionalLightData& light,
        const CameraData& camera,
        uint32_t shadowSRVIndex,
        const EnvironmentResource* envRes);

    void BindPipeline(CommandList& cmd, const PipelineState& pipelineState);

    void Draw(
        CommandList& cmd,
        MeshResource& mesh,
        MaterialResource& material,
        const Core::Matrix& world);

private:
    bool CreateRootSignature(Device& device);
    bool CreateDefaultPSOs();
    ID3D12PipelineState* CreatePSO(const PipelineState& pipelineState);
    ID3D12PipelineState* GetPipeline(const PipelineState& pipelineState);

    D3D12_GPU_VIRTUAL_ADDRESS UploadFrameCB(
        const DirectionalLightData& light,
        const CameraData& camera,
        uint32_t shadowSRVIndex,
        const EnvironmentResource* envRes);
    D3D12_GPU_VIRTUAL_ADDRESS UploadObjectCB(const Core::Matrix& world);
    D3D12_GPU_VIRTUAL_ADDRESS UploadMaterialCB(MaterialResource& material);

private:
    SurfaceRendererConfig m_config;
    PipelineCache& m_pipelineCache;
    ComPtr<ID3D12RootSignature> m_rootSignature;

    FrameConstantAllocator m_objectCBAllocator;
    FrameConstantAllocator m_materialCBAllocator;
    FrameConstantAllocator m_frameCBAllocator;

    D3D12_GPU_VIRTUAL_ADDRESS m_frameCBAddress{};
};