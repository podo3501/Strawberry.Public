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