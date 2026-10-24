module;

#include "d3dx12.h"

export module DxRender.Pipeline.Renderer:PipelineCache;

import std;
import DxRender.Core;
import DxRender.Definition;
import DxRender.Shader;

using Microsoft::WRL::ComPtr;

export class PipelineCache
{
public:
    ~PipelineCache() = default;
    PipelineCache(Device& device, ShaderLibrary& shaderLibrary);

    ID3D12PipelineState* GetOrCreate(
        const PipelineState& pipelineState,
        ID3D12RootSignature* rootSignature,
        const std::function<void(D3D12_GRAPHICS_PIPELINE_STATE_DESC&)>& setup);
    
    ID3D12PipelineState* Find(const PipelineState& pipelineState);

private:
    Device& m_device;
    ShaderLibrary& m_shaderLibrary;

    std::unordered_map<PipelineState, ComPtr<ID3D12PipelineState>, PipelineStateHasher> m_cache;
};