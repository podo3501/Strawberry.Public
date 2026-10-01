module;

#include <wrl/client.h>
#include "d3dx12.h"

export module Pipeline.Renderer:PipelineCache;

import std;
import Core.Assert;
import Runtime.Render.Core;
import Runtime.Render.Helper;
import Runtime.Render.Definition;
import Runtime.Render.Shader;

using Microsoft::WRL::ComPtr;

export class PipelineCache
{
public:
    ~PipelineCache() = default;
    PipelineCache(Device& device, ShaderLibrary& shaderLibrary) :
        m_device{ device },
        m_shaderLibrary{ shaderLibrary }
    {}

    ID3D12PipelineState* GetOrCreate(
        const PipelineState& pipelineState,
        ID3D12RootSignature* rootSignature,
        const std::function<void(D3D12_GRAPHICS_PIPELINE_STATE_DESC&)>& setup)
    {
        auto it = m_cache.find(pipelineState);
        if (it != m_cache.end())
            return it->second.Get();

        const ShaderEntry* shaderEntry = m_shaderLibrary.Find(pipelineState.shaderVariant);
        Core::Assert(shaderEntry);
        if (!shaderEntry)
            return nullptr;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso{};

        pso.InputLayout = { nullptr, 0 };
        pso.pRootSignature = rootSignature;
        pso.VS = { shaderEntry->vs->GetBufferPointer(), shaderEntry->vs->GetBufferSize() };
        pso.PS = { shaderEntry->ps->GetBufferPointer(), shaderEntry->ps->GetBufferSize() };

        CD3DX12_RASTERIZER_DESC raster(D3D12_DEFAULT);

        raster.FillMode = ToD3D12(pipelineState.rasterState.fillMode);
        raster.CullMode = ToD3D12(pipelineState.rasterState.cullMode);

        pso.RasterizerState = raster;
        pso.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        pso.SampleMask = UINT_MAX;
        pso.PrimitiveTopologyType = ToD3D12_PSO(pipelineState.topologyType);
        pso.NumRenderTargets = 1;
        pso.RTVFormats[0] = RenderFormat::BackBufferFormat;
        pso.SampleDesc.Count = 1;

        setup(pso);

        ComPtr<ID3D12PipelineState> pipeline;

        auto result = m_device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&pipeline));
        Core::Assert(SUCCEEDED(result));
        m_cache.emplace(pipelineState, pipeline);

        return pipeline.Get();
    }

    ID3D12PipelineState* Find(const PipelineState& pipelineState)
    {
        auto it = m_cache.find(pipelineState);
        if (it == m_cache.end())
            return nullptr;

        return it->second.Get();
    }

private:
    Device& m_device;
    ShaderLibrary& m_shaderLibrary;

    std::unordered_map<PipelineState, ComPtr<ID3D12PipelineState>, PipelineStateHasher> m_cache;
};