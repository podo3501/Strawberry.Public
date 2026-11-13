module;

#include "d3dx12.h"

module DxRender.Provider;

import :TextureUtils;
import DxRender.Core;
import DxRender.Helper;
import DxRender.Command;
import DxRender.Allocator;
import DxRender.Resource;
import DxRender.Factory;
import Contract.Render.Assets;
import Core.Logger;

namespace
{
    bool IsUAVCompatibleFormat(DXGI_FORMAT format) noexcept
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

    void ApplyMipSettings(D3D12_RESOURCE_DESC& desc, bool canGenerateMips) noexcept
    {
        if (!canGenerateMips) return;

        desc.MipLevels = 0; // full mip chain
        desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }
}

D3D12_RESOURCE_DESC CreateTexture2DDesc(const TextureAsset& asset, bool mips)
{
    auto texDesc = CreateTextureDescriptor(asset.size.width, asset.size.height, ToDXGIFormat(asset.format));
    ApplyMipSettings(texDesc, mips);

    return texDesc;
}

D3D12_RESOURCE_DESC CreateTextureCubeDesc(const TextureCubeAsset& asset)
{
    auto texDesc = CreateTextureDescriptor(asset.width, asset.height, ToDXGIFormat(asset.format));
    texDesc.DepthOrArraySize = 6; // 큐브맵 = 6면 배열
    texDesc.MipLevels = static_cast<UINT16>(asset.mipCount); // cmgen이 이미 구운 밉체인 그대로 사용 (0=full chain 아님)
    // UAV 플래그 불필요 - 런타임 밉 생성 없음
    return texDesc;
}

bool CanGenerateMips(const TextureAsset& asset, bool generateMips)
{
    DXGI_FORMAT format = ToDXGIFormat(asset.format);
    return generateMips && IsUAVCompatibleFormat(format);
}

bool IsValidTextureAsset(const TextureAsset& asset)
{
    if (asset.format == PixelFormat::Unknown)
    {
        Core::Logger::Error("TextureUtils: 텍스처 에셋의 형식을 알 수 없다");
        return false;
    }
    if (asset.size.width <= 0 || asset.size.height <= 0)
    {
        Core::Logger::Error("TextureUtils: 텍스처 에셋의 크기가 0 이하이다 ({}x{})", asset.size.width, asset.size.height);
        return false;
    }

    const auto requiredBytes = static_cast<std::uint64_t>(asset.stride) * static_cast<std::uint64_t>(asset.size.height);
    if (asset.stride == 0 || asset.pixels.size() < requiredBytes)
    {
        Core::Logger::Error("TextureUtils: 텍스처 에셋의 픽셀이 모자라다 (줄 간격 {}, 높이 {}, 픽셀 {}바이트)",
            asset.stride, asset.size.height, asset.pixels.size());
        return false;
    }
    return true;
}

bool IsValidTextureCubeAsset(const TextureCubeAsset& asset)
{
    if (asset.format == PixelFormat::Unknown)
    {
        Core::Logger::Error("TextureUtils: 큐브 텍스처 에셋의 형식을 알 수 없다");
        return false;
    }
    if (asset.width == 0 || asset.height == 0 || asset.mipCount == 0)
    {
        Core::Logger::Error("TextureUtils: 큐브 텍스처 에셋의 크기나 밉 개수가 0이다 ({}x{}, 밉 {})", asset.width, asset.height, asset.mipCount);
        return false;
    }
    if (asset.faceCount != 6)
    {
        Core::Logger::Error("TextureUtils: 큐브 텍스처 에셋의 면 개수가 6이 아니다 ({})", asset.faceCount);
        return false;
    }

    const auto expected = static_cast<std::size_t>(asset.mipCount) * asset.faceCount;
    if (asset.subImages.size() != expected)
    {
        Core::Logger::Error("TextureUtils: 큐브 텍스처 에셋의 이미지 개수가 밉 x 면과 다르다 ({} != {})", asset.subImages.size(), expected);
        return false;
    }
    for (const auto& sub : asset.subImages)
    {
        if (sub.rowPitch == 0 || sub.pixels.empty())
        {
            Core::Logger::Error("TextureUtils: 큐브 텍스처 에셋에 비어 있는 이미지가 있다");
            return false;
        }
    }
    return true;
}

void UploadTexture(
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

void UploadTextureCube(
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

bool CreateTextureViews(
    DescriptorFactory& descFactory,
    TextureResource* texRes,
    bool generateMips,
    std::vector<UINT>* outMipSrvIndices,
    std::vector<UINT>* outMipUavIndices)
{
    if (!texRes || !texRes->Get())
    {
        Core::Logger::Error("TextureUtils::CreateTextureViews: 텍스처 리소스가 없다");
        return false;
    }

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

bool CreateTextureCubeViews(
    DescriptorFactory& descFactory,
    TextureCubeResource* texRes)
{
    if (!texRes || !texRes->Get())
    {
        Core::Logger::Error("TextureUtils::CreateTextureCubeViews: 큐브 텍스처 리소스가 없다");
        return false;
    }

    auto& res = texRes->Get();
    const auto& resDesc = res->GetDesc();
    const UINT mipCount = resDesc.MipLevels;

    DXGI_FORMAT srvFormat = resDesc.Format; // 큐브맵은 항상 Linear -> sRGB 변환 불필요
    UINT index = descFactory.CreateTextureCubeSRV(res, srvFormat, mipCount);
    if (index == UINT_MAX) return false;

    texRes->SetHeapIndex(index);
    return true;
}