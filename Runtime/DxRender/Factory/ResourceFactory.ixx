module;

#include "d3dx12.h"

export module DxRender.Factory:Resource;

import DxRender.Core;

export enum class ResInitType
{
    Upload,
    Default,
    Readback
};

export class ResourceFactory
{
public:
    ~ResourceFactory() = default;
    ResourceFactory() = delete;
    explicit ResourceFactory(Device& device);

    Resource CreateTextureResource(const D3D12_RESOURCE_DESC& desc);
    Resource CreateResource(UINT64 size, ResInitType type);

    UINT64 GetRequiredIntermediateSize(
        const D3D12_RESOURCE_DESC& desc,
        UINT firstSubresource,
        UINT numSubresources,
        UINT64 offset);

private:
    Device& m_device;
};