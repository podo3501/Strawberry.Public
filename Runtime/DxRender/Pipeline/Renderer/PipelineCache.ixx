module;

#include "d3dx12.h"

export module DxRender.Pipeline.Renderer:PipelineCache;

import std;
import DxRender.Core;
import DxRender.Definition;
import DxRender.Shader;

using Microsoft::WRL::ComPtr;

// 키는 PipelineState뿐이라 같은 상태는 항상 같은 루트 시그니처로 부른다. 병렬 기록 중 여러 스레드가 함께 부른다.
// 만들지 못한 상태는 기억해서 다시 만들지도 오류를 또 남기지도 않고 nullptr을 돌려준다.
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
    ComPtr<ID3D12PipelineState> Build(
        const PipelineState& pipelineState,
        ID3D12RootSignature* rootSignature,
        const std::function<void(D3D12_GRAPHICS_PIPELINE_STATE_DESC&)>& setup);

    Device& m_device;
    ShaderLibrary& m_shaderLibrary;

    std::shared_mutex m_mutex; // 찾기는 함께, 만들기는 혼자
    std::unordered_map<PipelineState, ComPtr<ID3D12PipelineState>, PipelineStateHasher> m_cache;
    std::unordered_set<PipelineState, PipelineStateHasher> m_failed;
};
