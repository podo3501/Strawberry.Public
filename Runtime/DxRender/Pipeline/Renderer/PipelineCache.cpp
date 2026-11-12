module;

#include "d3dx12.h"

module DxRender.Pipeline.Renderer;

import :PipelineCache;
import Core.Logger;
import DxRender.Helper;
import DxRender.Definition;

using Microsoft::WRL::ComPtr;

PipelineCache::PipelineCache(Device& device, ShaderLibrary& shaderLibrary) :
    m_device{ device },
    m_shaderLibrary{ shaderLibrary }
{}

ID3D12PipelineState* PipelineCache::GetOrCreate(
    const PipelineState& pipelineState,
    ID3D12RootSignature* rootSignature,
    const std::function<void(D3D12_GRAPHICS_PIPELINE_STATE_DESC&)>& setup)
{
    {
        std::shared_lock readLock{ m_mutex };
        if (auto it = m_cache.find(pipelineState); it != m_cache.end())
            return it->second.Get();

        if (m_failed.contains(pipelineState))
            return nullptr;
    }

    std::unique_lock writeLock{ m_mutex };

    // 기다리는 동안 다른 스레드가 만들었거나 실패했을 수 있으니 다시 확인한다
    if (auto it = m_cache.find(pipelineState); it != m_cache.end())
        return it->second.Get();

    if (m_failed.contains(pipelineState))
        return nullptr;

    ComPtr<ID3D12PipelineState> pipeline = Build(pipelineState, rootSignature, setup);
    if (!pipeline)
    {
        m_failed.insert(pipelineState);
        return nullptr;
    }

    return m_cache.emplace(pipelineState, std::move(pipeline)).first->second.Get();
}

ID3D12PipelineState* PipelineCache::Find(const PipelineState& pipelineState)
{
    std::shared_lock readLock{ m_mutex };

    auto it = m_cache.find(pipelineState);
    if (it == m_cache.end())
        return nullptr;

    return it->second.Get();
}

ComPtr<ID3D12PipelineState> PipelineCache::Build(
    const PipelineState& pipelineState,
    ID3D12RootSignature* rootSignature,
    const std::function<void(D3D12_GRAPHICS_PIPELINE_STATE_DESC&)>& setup)
{
    const ShaderID shaderID = pipelineState.shaderVariant.shaderID;

    const ShaderEntry* shaderEntry = m_shaderLibrary.Find(pipelineState.shaderVariant);
    if (!shaderEntry)
    {
        // 컴파일에 실패했다면 그 원인은 셰이더 라이브러리가 이미 남겼다
        Core::Logger::Error("PipelineCache::Build: 셰이더 항목이 없다 (등록하지 않았거나 컴파일에 실패했다, shaderID={})", shaderID);
        return nullptr;
    }

    if (!shaderEntry->vs || !shaderEntry->ps)
    {
        Core::Logger::Error("PipelineCache::Build: 정점이나 픽셀 셰이더가 없다 (shaderID={}, vs={}, ps={})",
            shaderID, shaderEntry->vs != nullptr, shaderEntry->ps != nullptr);
        return nullptr;
    }

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

    HRESULT hr = m_device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&pipeline));
    if (FAILED(hr))
    {
        Core::Logger::Error("PipelineCache::Build: 파이프라인 상태를 만들지 못했다 (shaderID={}, hr={:#010x})",
            shaderID, static_cast<std::uint32_t>(hr));
        return nullptr;
    }

    return pipeline;
}
