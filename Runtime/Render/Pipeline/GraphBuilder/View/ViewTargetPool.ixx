module;

#include <d3d12.h>

export module Pipeline.GraphBuilder:ViewTargetPool;

import std;
import Runtime.Render.Release;
import Runtime.Render.Factory;
import Runtime.Render.Resource;
import Runtime.Render.Task;
import Runtime.Render.Core;
import Runtime.Render.RGResourceID;
import Runtime.Render.Definition;
import Runtime.Render.Helper;
import Core.Assert;
import Core.Math;
import Client.Render.View;

Resource CreateColorTarget(Device& device, const Core::Size& size)
{
    auto desc = CreateTextureDescriptor(size.width, size.height, RenderFormat::BackBufferFormat);
    desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format = RenderFormat::BackBufferFormat;
    clearValue.Color[0] = clearValue.Color[1] = clearValue.Color[2] = clearValue.Color[3] = 0.0f; // PMA 컨벤션: 완전 투명 = RGBA 모두 0

    return device.CreateResource(
        desc,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        &clearValue);
}

Resource CreateDepthTarget(Device& device, const Core::Size& size)
{
    auto desc = CreateTextureDescriptor(size.width, size.height, DXGI_FORMAT_R32_TYPELESS);
    desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format = RenderFormat::DepthFormat;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    return device.CreateResource(
        desc,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue);
}

export class ViewTargetPool
{
public:
    ~ViewTargetPool() = default;
    ViewTargetPool() = delete;

    ViewTargetPool(
        Device& device,
        TaskScheduler& taskScheduler,
        RGResourceIDAllocator& idAllocator,
        DescriptorFactory& descFactory) noexcept :
        m_device{ device },
        m_idAllocator{ idAllocator },
        m_descFactory{ descFactory },
        m_deferredReleaser{ taskScheduler }
    {}

    ViewTargetResource& Acquire(ViewID id, const Core::Size& requiredSize)
    {
        auto index = static_cast<std::size_t>(id);
        auto& view = m_views[index];
        if (view)
        {
            if (view->GetSize() == requiredSize)
                return *view;

            m_deferredReleaser.Add(view);
            view.reset();
        }

        view = CreateViewTargetResource(requiredSize);
        return *view;
    }

    void ApplyResourceBindings(ResourceContext& resCtx) const
    {
        for (const auto& view : m_views)
        {
            if (!view) continue;

            resCtx.Set(view->GetColorID(), view->GetColorResource());
            resCtx.Set(view->GetDepthID(), view->GetDepthResource());
        }
    }

    void PruneUnused(const std::bitset<MaxViewCount>& activeViews)
    {
        for (std::size_t i = 0; i < m_views.size(); ++i)
        {
            auto& view = m_views[i];
            if (!view)
                continue;

            if (activeViews.test(i))
                continue;

            m_deferredReleaser.Add(view);
            view.reset();
        }
    }

    void Update()
    {
        m_deferredReleaser.Flush();
    }

private:
    std::shared_ptr<ViewTargetResource> CreateViewTargetResource(const Core::Size& size)
    {
        ViewTargetResourceDesc desc;
        desc.size = size;
        desc.color = CreateColorTarget(m_device, size);
        desc.depth = CreateDepthTarget(m_device, size);

        if (desc.color && desc.depth)
        {
            desc.colorID = m_idAllocator.AllocateDynamic();
            desc.depthID = m_idAllocator.AllocateDynamic();

            desc.colorRTVIndex = m_descFactory.CreateTextureRTV(desc.color, RenderFormat::BackBufferFormat);
            desc.depthDSVIndex = m_descFactory.CreateTextureDSV(desc.depth, DXGI_FORMAT_D32_FLOAT);
            desc.heapIndex = m_descFactory.CreateTextureSRV(desc.color, RenderFormat::BackBufferFormat);
        }

        Core::Assert(desc.colorRTVIndex != std::numeric_limits<std::uint32_t>::max() &&
            desc.depthDSVIndex != std::numeric_limits<std::uint32_t>::max() &&
            desc.heapIndex != std::numeric_limits<std::uint32_t>::max());

        auto view = std::make_shared<ViewTargetResource>(
            std::move(desc),
            [this](const ViewTargetResource& r) { ReleaseViews(r); });

        return view;
    }

    void ReleaseViews(const ViewTargetResource& res)
    {
        constexpr auto invalidIndex = std::numeric_limits<std::uint32_t>::max();

        if (res.GetColorRTVIndex() != invalidIndex) m_descFactory.FreeRTV(res.GetColorRTVIndex());
        if (res.GetDepthDSVIndex() != invalidIndex) m_descFactory.FreeDSV(res.GetDepthDSVIndex());

        if (res.GetColorID() != InvalidRGID) m_idAllocator.FreeDynamic(res.GetColorID());
        if (res.GetDepthID() != InvalidRGID) m_idAllocator.FreeDynamic(res.GetDepthID());
    }

    Device& m_device;
    RGResourceIDAllocator& m_idAllocator;
    DescriptorFactory& m_descFactory;
    DeferredReleaser m_deferredReleaser;

    std::array<std::shared_ptr<ViewTargetResource>, MaxViewCount> m_views{};
};