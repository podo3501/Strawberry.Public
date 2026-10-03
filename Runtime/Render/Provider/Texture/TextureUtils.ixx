module;

#include "d3dx12.h"

export module Runtime.Render.Provider:TextureUtils;

import std;
import Runtime.Render.Core;
import Runtime.Render.Command;
import Runtime.Render.Helper;
import Runtime.Render.Resource;
import Runtime.Render.Helper;
import Runtime.Render.Allocator;
import Runtime.Render.Factory;
import Client.Asset.Data;

inline bool IsUAVCompatibleFormat(DXGI_FORMAT format) noexcept
{
    switch (format)
    {
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_SINT:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SINT:
    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
        return true;
    default:
        return false;
    }
}

inline void ApplyMipSettings(D3D12_RESOURCE_DESC& desc, bool canGenerateMips) noexcept
{
    if (!canGenerateMips) return;

    desc.MipLevels = 0; // full mip chain
    desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
}

export D3D12_RESOURCE_DESC CreateTexture2DDesc(const TextureAsset& asset, bool mips)
{
    auto texDesc = CreateTextureDescriptor(asset.size.width, asset.size.height, ToDXGIFormat(asset.format));
    ApplyMipSettings(texDesc, mips);

    return texDesc;
}

export D3D12_RESOURCE_DESC CreateTextureCubeDesc(const TextureCubeAsset& asset)
{
    auto texDesc = CreateTextureDescriptor(asset.width, asset.height, ToDXGIFormat(asset.format));
    texDesc.DepthOrArraySize = 6; // 큐브맵 = 6면 배열
    texDesc.MipLevels = static_cast<UINT16>(asset.mipCount); // cmgen이 이미 구운 밉체인 그대로 사용 (0=full chain 아님)
    // UAV 플래그 불필요 - 런타임 밉 생성 없음
    return texDesc;
}

export bool CanGenerateMips(const TextureAsset& asset, bool generateMips)
{
    DXGI_FORMAT format = ToDXGIFormat(asset.format);
    return generateMips && IsUAVCompatibleFormat(format);
}

export void UploadTexture(
    CommandList& uploadCmd,
    const TextureAsset& asset,
    Resource& texRes,
    Resource& uploadRes,
    std::uint64_t offset)
{
    D3D12_SUBRESOURCE_DATA subresource{};
    subresource.pData = asset.pixels.data();
    subresource.RowPitch = asset.stride;
    subresource.SlicePitch = asset.stride * asset.size.height;

    UpdateSubresources(
        uploadCmd.Get(),
        texRes.Get(),
        uploadRes.Get(),
        offset,
        0,
        1,
        &subresource
    );
}

export void UploadTextureCube(
    CommandList& uploadCmd,
    const TextureCubeAsset& asset,
    Resource& texRes,
    Resource& uploadRes,
    std::uint64_t offset)
{
    const std::uint32_t subresourceCount = asset.mipCount * asset.faceCount;
    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
    subresources.reserve(subresourceCount);

    // D3D12 서브리소스 순서 규칙: mip이 가장 안쪽, 그다음 face(array slice)
    // subresourceIndex = mip + face * mipCount (기존 asset.subImages 인덱싱과 동일한 규칙)
    for (std::uint32_t face = 0; face < asset.faceCount; ++face)
    {
        for (std::uint32_t mip = 0; mip < asset.mipCount; ++mip)
        {
            const auto& sub = asset.subImages[mip + face * asset.mipCount];

            D3D12_SUBRESOURCE_DATA subresource{};
            subresource.pData = sub.pixels.data();
            subresource.RowPitch = sub.rowPitch;
            subresource.SlicePitch = static_cast<LONG_PTR>(sub.pixels.size());

            subresources.push_back(subresource);
        }
    }

    UpdateSubresources(
        uploadCmd.Get(),
        texRes.Get(),
        uploadRes.Get(),
        offset,
        0,
        subresourceCount,
        subresources.data()
    );
}

export bool CreateTextureViews(
    DescriptorFactory& descFactory,
    TextureResource* texRes,
    bool generateMips,
    std::vector<UINT>* outMipSrvIndices,
    std::vector<UINT>* outMipUavIndices)
{
    if (!texRes) return false;

    auto& res = texRes->Get();
    const auto& resDesc = res->GetDesc();
    const UINT mipCount = resDesc.MipLevels;

    DXGI_FORMAT srvFormat = resDesc.Format;
    if (texRes->GetDesc().colorSpace == ColorSpace::SRGB)
        srvFormat = ToSRGB(resDesc.Format);

    UINT mainMipLevels = generateMips ? mipCount : 1;
    UINT mainIndex = descFactory.CreateTextureSRV(res, srvFormat, mainMipLevels);
    if (mainIndex == UINT_MAX) return false;

    texRes->SetHeapIndex(mainIndex);

    if (generateMips && mipCount > 1)
    {
        if (outMipSrvIndices) outMipSrvIndices->reserve(mipCount);
        if (outMipUavIndices) outMipUavIndices->reserve(mipCount);

        for (UINT i = 0; i < mipCount; ++i)
        {
            UINT mipSrvIndex = descFactory.CreateMipSRV(res, srvFormat, i);
            if (mipSrvIndex == UINT_MAX) return false;
            if (outMipSrvIndices) outMipSrvIndices->push_back(mipSrvIndex);

            UINT mipUavIndex = descFactory.CreateMipUAV(res, resDesc.Format, i);
            if (mipUavIndex == UINT_MAX) return false;
            if (outMipUavIndices) outMipUavIndices->push_back(mipUavIndex);
        }
    }

    return true;
}

export bool CreateTextureCubeViews(
    DescriptorFactory& descFactory,
    TextureCubeResource* texRes)
{
    if (!texRes) return false;

    auto& res = texRes->Get();
    const auto& resDesc = res->GetDesc();
    const UINT mipCount = resDesc.MipLevels;

    DXGI_FORMAT srvFormat = resDesc.Format; // 큐브맵은 항상 Linear -> sRGB 변환 불필요
    UINT index = descFactory.CreateTextureCubeSRV(res, srvFormat, mipCount);
    if (index == UINT_MAX) return false;

    texRes->SetHeapIndex(index);
    return true;
}