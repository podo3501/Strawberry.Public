module;

#include <d3d12.h>
#include <wrl.h>
#include <dxcapi.h>

export module DxRender.Provider:MipGenerator;

import std;
import Core.Utils;
import Pipeline.Renderer;
import DxRender.Core;
import DxRender.Shader;
import DxRender.Command;
import DxRender.Resource;
import DxRender.Definition;
import Contract.Asset.Data;
import Contract.Render.Definition;

using Microsoft::WRL::ComPtr;

export enum class MipType : std::uint8_t
{
    SRGB = 0,
    Data,
    Count
};

struct MipShaderDesc
{
    MipType type;
    std::vector<ShaderMacroDesc> macros;
};

inline MipType GetMipType(ColorSpace colorSpace)
{
    switch (colorSpace)
    {
    case ColorSpace::SRGB: return MipType::SRGB;
    case ColorSpace::Linear: return MipType::Data;
    default: return MipType::Data;
    }
}

export class MipGenerator
{
public:
    ~MipGenerator() = default;

    explicit MipGenerator(Device& device) : 
        m_device{ device }
    {}

    bool Initialize(ShaderLibrary& shaderLibrary)
    {
        if (!LoadShader(shaderLibrary)) return false;
        if (!CreateRootSignature()) return false;
        if (!CreatePSO()) return false;

        return true;
    }

    void GenerateMips(
        CommandList& cmd,
        TextureResource* texResource,
        const std::vector<UINT>& mipSrvIndices,
        const std::vector<UINT>& mipUavIndices)
    {
        if (!texResource)
            return;

        auto& texRes = texResource->Get();
        const D3D12_RESOURCE_DESC desc = texRes->GetDesc();
        const UINT mipCount = desc.MipLevels;

        if (mipCount <= 1)
            return;

        cmd->SetComputeRootSignature(m_rootSignature.Get());

        const MipType mipType = GetMipType(texResource->GetDesc().colorSpace);
        auto* pso = GetPSO(mipType);
        if (!pso) return;
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

            cmd->Dispatch((width + 7) / 8, (height + 7) / 8, 1);

            D3D12_RESOURCE_BARRIER barrier{};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barrier.UAV.pResource = texRes.Get();
            cmd->ResourceBarrier(1, &barrier);
        }
    }

private:
    enum class RootSlot : std::uint32_t
    {
        Constants = 0
    };

    bool LoadShader(ShaderLibrary& shaderLibrary)
    {
        static const MipShaderDesc shaders[] =
        {
            { MipType::SRGB,{} },
            { MipType::Data,{ { "IS_DATA_MAP" } } }
        };

        std::size_t loadedCount = 0;
        for (auto& desc : shaders)
        {
            ShaderVariant variant{ RegistryShader::MipGenerator };
            variant.runtimeMacros = desc.macros;

            if (const auto* entry = shaderLibrary.Find(variant))
            {
                m_shaderBlobs[Core::ToIndex(desc.type)] = entry->cs;
                ++loadedCount;
            }
        }

        return loadedCount == std::size(shaders);
    }

    bool CreateRootSignature()
    {
        RootSignatureBuilder builder;
        builder.Add32BitConstants(Core::ToIndex(RootSlot::Constants), 4);

        m_rootSignature = builder.Build(m_device);
        return m_rootSignature != nullptr;
    }

    bool CreatePSO()
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

            if (FAILED(m_device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&m_psoMap[i])))) return false;
        }

        return true;
    }

    ID3D12PipelineState* GetPSO(MipType type) const
    {
        return m_psoMap[Core::ToIndex(type)].Get();
    }

    Device& m_device;
    ComPtr<ID3D12RootSignature> m_rootSignature;

    std::array<ComPtr<IDxcBlob>, Core::EnumSize<MipType>> m_shaderBlobs;
    std::array<ComPtr<ID3D12PipelineState>, Core::EnumSize<MipType>> m_psoMap;
};