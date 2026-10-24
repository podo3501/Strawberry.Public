module;

#include "d3dx12.h"

export module DxRender.Pipeline.Renderer:Composite;

import std;
import :PipelineCache;
import DxRender.Core;
import DxRender.Definition;
import DxRender.Command;

export class CompositeRenderer
{
public:
    ~CompositeRenderer() = default;
    CompositeRenderer(PipelineCache& pipelineCache);

    bool Initialize(Device& device);
    void PrepareDraw(CommandList& cmd);
    void Draw(CommandList& cmd, UINT colorSRVIndex);

private:
    bool CreateRootSignature(Device& device);
    ID3D12PipelineState* CreatePSO(const PipelineState& pipelineState);

    PipelineCache& m_pipelineCache;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    ID3D12PipelineState* m_compositePSO{ nullptr };
};