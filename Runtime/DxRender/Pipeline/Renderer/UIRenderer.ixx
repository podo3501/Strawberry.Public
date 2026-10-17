module;

#include "d3dx12.h"

export module Pipeline.Renderer:UI;

import std;
import :Config;
import :PipelineCache;
import Core.Math;
import DxRender.Core;
import DxRender.Command;
import DxRender.Resource;
import DxRender.Allocator;
import DxRender.Definition;

using Microsoft::WRL::ComPtr;

export class UIRenderer
{
public:
    ~UIRenderer() = default;
    UIRenderer(const UIRendererConfig& config, PipelineCache& pipelineCache);

    bool Initialize(Device& device);
    void ResetFrameResources(uint32_t slot);
    void BeginFrame(CommandList& cmd);
    void Draw(
        CommandList& cmd,
        MeshResource& mesh,
        const Core::Matrix& viewProj);

private:
    bool CreateRootSignature(Device& device);
    ID3D12PipelineState* CreatePSO();
    D3D12_GPU_VIRTUAL_ADDRESS UploadDrawCB(const Core::Matrix& viewProj);

    UIRendererConfig m_config;
    PipelineCache& m_pipelineCache;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ID3D12PipelineState* m_pso{ nullptr };

    FrameConstantAllocator m_uiDrawCBAllocator;
};