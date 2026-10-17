module;

#include <wrl.h>
#include <DirectXMath.h>
#include "d3dx12.h"

export module Pipeline.Renderer:Shadow;

import std;
import :Config;
import :PipelineCache;
import Core.Math;
import DxRender.Core;
import DxRender.Definition;
import DxRender.Allocator;
import DxRender.Command;
import DxRender.Resource;
import Contract.Render.View;

export class ShadowRenderer
{
public:
    ~ShadowRenderer() = default;
    ShadowRenderer() = delete;
    ShadowRenderer(const ShadowRendererConfig& config, PipelineCache& pipelineCache);

    bool Initialize(Device& device);
    void ResetFrameResources(std::uint32_t slot);
    void PrepareDraw(CommandList& cmd, const DirectionalLightData& light);
    void Draw(CommandList& cmd, MeshResource& mesh, const Core::Matrix& world);

private:
    bool CreateRootSignature(Device& device);
    ID3D12PipelineState* CreatePSO(const PipelineState& pipelineState);
    D3D12_GPU_VIRTUAL_ADDRESS UploadFrameCB(const DirectionalLightData& light);
    D3D12_GPU_VIRTUAL_ADDRESS UploadObjectCB(const Core::Matrix& world);

    ShadowRendererConfig m_config;
    PipelineCache& m_pipelineCache;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    ID3D12PipelineState* m_shadowPSO{ nullptr };

    FrameConstantAllocator m_objectCBAllocator;
    FrameConstantAllocator m_frameCBAllocator;
};