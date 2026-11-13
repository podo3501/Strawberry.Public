module;

#include "d3dx12.h"

export module DxRender.Pipeline.Renderer:UI;

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
    // 상수 버퍼의 한도를 넘으면 false이고 그리지 않는다. 부르는 쪽이 그 항목을 건너뛴다.
    [[nodiscard]] bool Draw(
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