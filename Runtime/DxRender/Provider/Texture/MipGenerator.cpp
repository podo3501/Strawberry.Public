module;

#include <d3d12.h>
#include <wrl.h>
#include <dxcapi.h>

module DxRender.Provider;

import std;
import :MipGenerator;
import DxRender.Core;
import DxRender.Definition;
import Core.Logger;
import Core.Utils;
import Contract.Render.Assets;
import Contract.Render.Backend.Types;

namespace
{
    enum class RootSlot : std::uint32_t
    {
        Constants = 0,
    };

    struct MipShaderDesc
    {
        MipType type;
        std::vector<ShaderMacroDesc> macros;
    };

    // 한 서브리소스의 상태를 바꾸는 배리어
    D3D12_RESOURCE_BARRIER MakeTransition(
        ID3D12Resource* resource,
        UINT subresource,
        D3D12_RESOURCE_STATES before,
        D3D12_RESOURCE_STATES after)
    {
        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = resource;
        barrier.Transition.Subresource = subresource;
        barrier.Transition.StateBefore = before;
        barrier.Transition.StateAfter = after;
        return barrier;
    }

    MipType GetMipType(ColorSpace colorSpace)
    {
        switch (colorSpace)
        {
        case ColorSpace::SRGB: return MipType::SRGB;
        case ColorSpace::Linear: return MipType::Data;
        default: return MipType::Data;
        }
    }
}

MipGenerator::MipGenerator(Device& device) :
    m_device{ device }
{}

bool MipGenerator::Initialize(ShaderLibrary& shaderLibrary)
{
    if (!LoadShader(shaderLibrary)) return false;
    if (!CreateRootSignature()) return false;
    if (!CreatePSO()) return false;

    return true;
}

void MipGenerator::GenerateMips(
    CommandList& cmd,
    TextureResource* texResource,
    const std::vector<UINT>& mipSrvIndices,
    const std::vector<UINT>& mipUavIndices)
{
    if (!texResource || !texResource->Get())
    {
        Core::Logger::Error("MipGenerator::GenerateMips: 텍스처 리소스가 없다");
        return;
    }

    auto& texRes = texResource->Get();
    const D3D12_RESOURCE_DESC desc = texRes->GetDesc();
    const UINT mipCount = desc.MipLevels;

    if (mipCount <= 1)
        return;

    if (mipSrvIndices.size() < mipCount || mipUavIndices.size() < mipCount)
    {
        Core::Logger::Error("MipGenerator::GenerateMips: 밉 뷰가 모자라다 (밉 {}, SRV {}, UAV {})",
            mipCount, mipSrvIndices.size(), mipUavIndices.size());
        return;
    }

    cmd->SetComputeRootSignature(m_rootSignature.Get());

    const MipType mipType = GetMipType(texResource->GetDesc().colorSpace);
    auto* pso = GetPSO(mipType);
    if (!pso)
    {
        Core::Logger::Error("MipGenerator::GenerateMips: 밉 종류 {}의 파이프라인이 없다", Core::ToIndex(mipType));
        return;
    }
    cmd.SetPipelineState(pso);

    for (UINT srcMip = 0; srcMip < mipCount - 1; ++srcMip)
    {
        UINT dstMip = srcMip + 1;

        UINT srcMipSrvIndex = mipSrvIndices[srcMip];
        UINT dstMipUavIndex = mipUavIndices[dstMip];

        UINT width = std::max(1u, static_cast<UINT>(desc.Width >> dstMip));
        UINT height = std::max(1u, static_cast<UINT>(desc.Height >> dstMip));

        std::uint32_t constants[4] = {
            srcMipSrvIndex,  // 셰이더에서 읽을 소스 SRV 인덱스
            dstMipUavIndex,  // 셰이더에서 쓸 목적지 UAV 인덱스
            width,
            height
        };
        cmd->SetComputeRoot32BitConstants(Core::ToIndex(RootSlot::Constants), 4, constants, 0);

        // 원본 밉은 SRV로 읽으므로 읽는 동안만 셰이더 리소스 상태로 둔다 (그래프가 보는 텍스처 전체 상태는 UAV 그대로)
        D3D12_RESOURCE_BARRIER toRead = MakeTransition(texRes.Get(), srcMip,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        cmd->ResourceBarrier(1, &toRead);

        cmd->Dispatch((width + 7) / 8, (height + 7) / 8, 1);

        D3D12_RESOURCE_BARRIER barriers[2]{};
        barriers[0] = MakeTransition(texRes.Get(), srcMip,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barriers[1].UAV.pResource = texRes.Get();
        cmd->ResourceBarrier(2, barriers);
    }
}

bool MipGenerator::LoadShader(ShaderLibrary& shaderLibrary)
{
    const MipShaderDesc shaders[] =
    {
        { MipType::SRGB,{} },
        { MipType::Data,{ { "IS_DATA_MAP" } } }
    };

    std::size_t loadedCount = 0;
    for (auto& desc : shaders)
    {
        ShaderVariant variant{ RegistryShader::MipGenerator };
        variant.SetRuntimeMacros(desc.macros);

        if (const auto* entry = shaderLibrary.Find(variant))
        {
            m_shaderBlobs[Core::ToIndex(desc.type)] = entry->cs;
            ++loadedCount;
        }
    }

    if (loadedCount != std::size(shaders))
    {
        Core::Logger::Error("MipGenerator::LoadShader: 밉 셰이더 {}개 중 {}개만 찾았다", std::size(shaders), loadedCount);
        return false;
    }
    return true;
}

bool MipGenerator::CreateRootSignature()
{
    RootSignatureBuilder builder;
    builder.Add32BitConstants(Core::ToIndex(RootSlot::Constants), 4);

    m_rootSignature = builder.Build(m_device);
    if (!m_rootSignature)
    {
        Core::Logger::Error("MipGenerator::CreateRootSignature: 루트 시그니처를 만들지 못했다");
        return false;
    }
    return true;
}

bool MipGenerator::CreatePSO()
{
    D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
    desc.pRootSignature = m_rootSignature.Get();

    for (std::size_t i = 0; i < Core::EnumSize<MipType>; ++i)
    {
        auto& blob = m_shaderBlobs[i];
        if (!blob)
            continue;

        desc.CS = {
            blob->GetBufferPointer(),
            blob->GetBufferSize()
        };

        const HRESULT hr = m_device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&m_psoMap[i]));
        if (FAILED(hr))
        {
            Core::Logger::Error("MipGenerator::CreatePSO: 밉 종류 {}의 파이프라인을 만들지 못했다 (hr={:#010x})",
                i, static_cast<std::uint32_t>(hr));
            return false;
        }
    }

    return true;
}

ID3D12PipelineState* MipGenerator::GetPSO(MipType type) const
{
    return m_psoMap[Core::ToIndex(type)].Get();
}