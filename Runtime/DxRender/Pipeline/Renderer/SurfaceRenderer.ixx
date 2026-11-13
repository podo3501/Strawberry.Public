module;

#include "d3dx12.h"

export module DxRender.Pipeline.Renderer:Surface;

import std;
import :Config;
import :PipelineCache;
import Core.Math;
import DxRender.Core;
import DxRender.Command;
import DxRender.Resource;
import DxRender.Allocator;
import DxRender.Definition;
import Contract.Render.Shared.View;

using Microsoft::WRL::ComPtr;

export class SurfaceRenderer
{
public:
    ~SurfaceRenderer() = default;
    SurfaceRenderer() = delete;
    SurfaceRenderer(const SurfaceRendererConfig& config, PipelineCache& pipelineCache);

    bool Initialize(Device& device);
    void ResetFrameResources(uint32_t slot);

    // 프레임 상수 버퍼의 한도를 넘으면 false이고 아무것도 정하지 않는다. 부르는 쪽이 그 목록의 그리기를 건너뛴다.
    [[nodiscard]] bool PrepareDraw(
        CommandList& cmd,
        const DirectionalLightData& light,
        const CameraData& camera,
        uint32_t shadowSRVIndex,
        const EnvironmentResource* envRes);

    // PSO를 얻지 못하면 false이고 아무것도 정하지 않는다. 부르는 쪽이 그 항목의 그리기를 건너뛴다.
    [[nodiscard]] bool BindPipeline(CommandList& cmd, const PipelineState& pipelineState);

    // 항목 상수 버퍼의 한도를 넘으면 false이고 그리지 않는다. 부르는 쪽이 나머지 항목을 건너뛴다.
    [[nodiscard]] bool Draw(
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