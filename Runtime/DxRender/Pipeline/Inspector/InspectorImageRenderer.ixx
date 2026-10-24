module;

#include "d3dx12.h"

export module DxRender.Pipeline.Inspector:ImageRenderer;

import std;
import Core.Math;
import DxRender.Pipeline.Renderer;
import DxRender.Core;
import DxRender.Command;
import DxRender.Allocator;
import DxRender.Definition;

export class InspectorImageRenderer
{
public:
    ~InspectorImageRenderer() = default;
    InspectorImageRenderer() = delete;
    explicit InspectorImageRenderer(PipelineCache& pipelineCache) noexcept;
    bool Initialize(Device& device, const Core::Size& screenSize);

    void PrepareFrame(std::uint32_t slot);
    void BeginFrame(CommandList& cmd);
    void BindPipeline(CommandList& cmd);

    void Draw(CommandList& cmd, UINT srvIndex);
    void SetScreenSize(const Core::Size& size);

private:
    bool CreateRootSignature(Device& device);
    bool CreateDefaultPSOs();
    ID3D12PipelineState* CreatePSO(const PipelineState& pipelineState);

    D3D12_GPU_VIRTUAL_ADDRESS UploadDrawCB();
    ID3D12PipelineState* GetPipeline(const PipelineState& pipelineState);

    PipelineState m_pipelineState;
    PipelineCache& m_pipelineCache;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

    Core::Size m_screenSize{};
    FrameConstantAllocator m_drawCBAllocator;
    Core::Matrix m_projection;
};