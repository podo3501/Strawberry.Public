module;

#include <d3d12.h>

module DxRender.Pipeline.GraphBuilder;

import :ViewTargetPool;
import DxRender.Core;
import DxRender.Definition;
import DxRender.Helper;
import Core.Logger;

namespace 
{
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
}

ViewTargetPool::ViewTargetPool(
    Device& device,
    TaskScheduler& taskScheduler,
    RGResourceIDAllocator& idAllocator,
    DescriptorFactory& descFactory) noexcept :
    m_device{ device },
    m_idAllocator{ idAllocator },
    m_descFactory{ descFactory },
    m_deferredReleaser{ taskScheduler }
{}

static_assert(MaxViewTargetSide == D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION);

ViewTargetResources* ViewTargetPool::Acquire(ViewID id, const Core::Size& requiredSize)
{
    if (id >= m_views.size())
    {
        Core::Logger::Error("ViewTargetPool::Acquire: 뷰 번호 {}가 범위(0~{})를 벗어났다", id, m_views.size() - 1);
        return nullptr;
    }
    if (requiredSize.width == 0 || requiredSize.height == 0 ||
        requiredSize.width > MaxViewTargetSide || requiredSize.height > MaxViewTargetSide)
    {
        Core::Logger::Error("ViewTargetPool::Acquire: 뷰 {}의 크기 {}x{}가 올바르지 않다 (1~{})",
            id, requiredSize.width, requiredSize.height, MaxViewTargetSide);
        return nullptr;
    }

    auto& view = m_views[static_cast<std::size_t>(id)];
    if (view)
    {
        if (view->GetSize() == requiredSize)
            return view.get();

        m_deferredReleaser.Add(view);
        view.reset();
    }

    view = CreateViewTargetResource(requiredSize);
    return view.get();
}

void ViewTargetPool::ApplyResourceBindings(ResourceContext& resCtx) const
{
    for (const auto& view : m_views)
    {
        if (!view) continue;

        resCtx.Set(view->GetColorID(), view->GetColorResource());
        resCtx.Set(view->GetDepthID(), view->GetDepthResource());
    }
}

void ViewTargetPool::PruneUnused(const std::bitset<MaxViewCount>& activeViews)
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

void ViewTargetPool::Update()
{
    m_deferredReleaser.Flush();
}

std::shared_ptr<ViewTargetResources> ViewTargetPool::CreateViewTargetResource(const Core::Size& size)
{
    ViewTargetResourceDesc desc;
    desc.size = size;
    desc.color = CreateColorTarget(m_device, size);
    desc.depth = CreateDepthTarget(m_device, size);

    if (!desc.color || !desc.depth)
        return nullptr; // 원인은 Device가 남김. 아직 받은 ID나 디스크립터가 없다

    desc.colorID = m_idAllocator.AllocateDynamic();
    desc.depthID = m_idAllocator.AllocateDynamic();
    desc.colorRTVIndex = m_descFactory.CreateTextureRTV(desc.color, RenderFormat::BackBufferFormat);
    desc.depthDSVIndex = m_descFactory.CreateTextureDSV(desc.depth, DXGI_FORMAT_D32_FLOAT);
    desc.heapIndex = m_descFactory.CreateTextureSRV(desc.color, RenderFormat::BackBufferFormat);

    constexpr auto invalidIndex = std::numeric_limits<std::uint32_t>::max();
    const bool complete =
        desc.colorID != InvalidRGID && desc.depthID != InvalidRGID &&
        desc.colorRTVIndex != invalidIndex && desc.depthDSVIndex != invalidIndex && desc.heapIndex != invalidIndex;

    // 일부만 받았어도 일단 감싼다: 만들지 못하고 사라질 때 받은 ID와 디스크립터는 ReleaseViews가 돌려준다 (GPU가 쓴 적이 없어 바로 돌려도 안전하다)
    auto view = std::make_shared<ViewTargetResources>(
        std::move(desc),
        [this](const ViewTargetResources& res) { ReleaseViews(res); });

    if (!complete)
    {
        Core::Logger::Error("ViewTargetPool: 뷰 타겟의 ID나 디스크립터를 얻지 못했다 ({}x{})", size.width, size.height);
        return nullptr;
    }

    return view;
}

void ViewTargetPool::ReleaseViews(const ViewTargetResources& res)
{
    constexpr auto invalidIndex = std::numeric_limits<std::uint32_t>::max();

    if (res.GetColorRTVIndex() != invalidIndex) m_descFactory.FreeRTV(res.GetColorRTVIndex());
    if (res.GetDepthDSVIndex() != invalidIndex) m_descFactory.FreeDSV(res.GetDepthDSVIndex());

    if (res.GetColorID() != InvalidRGID) m_idAllocator.FreeDynamic(res.GetColorID());
    if (res.GetDepthID() != InvalidRGID) m_idAllocator.FreeDynamic(res.GetDepthID());
}