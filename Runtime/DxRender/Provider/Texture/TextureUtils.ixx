module;

#include "d3dx12.h"

export module DxRender.Provider:TextureUtils;

import std;
import DxRender.Core;
import DxRender.Command;
import DxRender.Resource;
import DxRender.Factory;
import Contract.Render.Assets;

export D3D12_RESOURCE_DESC CreateTexture2DDesc(const TextureAsset& asset, bool mips);
export D3D12_RESOURCE_DESC CreateTextureCubeDesc(const TextureCubeAsset& asset);
export bool CanGenerateMips(const TextureAsset& asset, bool generateMips);

// 형식, 크기, 줄 간격, 픽셀 바이트 수가 업로드할 수 있는 에셋인지 확인한다 (아니면 이유와 함께 Error)
export bool IsValidTextureAsset(const TextureAsset& asset);
export bool IsValidTextureCubeAsset(const TextureCubeAsset& asset);

export void UploadTexture(
    CommandList& uploadCmd,
    const TextureAsset& asset,
    Resource& texRes,
    Resource& uploadRes,
    std::uint64_t offset);

export void UploadTextureCube(
    CommandList& uploadCmd,
    const TextureCubeAsset& asset,
    Resource& texRes,
    Resource& uploadRes,
    std::uint64_t offset);

export bool CreateTextureViews(
    DescriptorFactory& descFactory,
    TextureResource* texRes,
    bool generateMips,
    std::vector<UINT>* outMipSrvIndices,
    std::vector<UINT>* outMipUavIndices);

export bool CreateTextureCubeViews(
    DescriptorFactory& descFactory,
    TextureCubeResource* texRes);