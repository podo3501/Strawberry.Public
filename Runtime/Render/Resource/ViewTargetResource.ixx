export module Runtime.Render.Resource:ViewTarget;

import std;
import Core.Math;
import Client.Render.Interfaces;
import Runtime.Render.Core;
import Runtime.Render.RGResourceID;

export struct ViewTargetResourceDesc
{
    Resource color;
    Resource depth;
    std::uint32_t colorRTVIndex{ std::numeric_limits<std::uint32_t>::max() };
    std::uint32_t depthDSVIndex{ std::numeric_limits<std::uint32_t>::max() };
    std::uint32_t heapIndex{ std::numeric_limits<std::uint32_t>::max() };
    RGResourceID colorID{};
    RGResourceID depthID{};
    Core::Size size{};
};

export class ViewTargetResource : public IResource
{
public:
    using ReleaseFn = std::function<void(const ViewTargetResource&)>;

    virtual ~ViewTargetResource() override
    {
        if (m_onRelease)
            m_onRelease(*this); // pending 이후 지워진 다음에 호출되어야 함.
    }

    ViewTargetResource(ViewTargetResourceDesc desc, ReleaseFn onRelease) noexcept :
        m_desc(std::move(desc)),
        m_onRelease(std::move(onRelease)),
        m_ready(true)
    {}

    virtual bool IsReady() const noexcept override { return m_ready; }

    const Core::Size& GetSize() const noexcept { return m_desc.size; }
    Resource& GetColorResource() noexcept { return m_desc.color; }
    Resource& GetDepthResource() noexcept { return m_desc.depth; }
    std::uint32_t GetColorRTVIndex() const noexcept { return m_desc.colorRTVIndex; }
    std::uint32_t GetDepthDSVIndex() const noexcept { return m_desc.depthDSVIndex; }
    std::uint32_t GetHeapIndex() const noexcept { return m_desc.heapIndex; }
    RGResourceID GetColorID() const noexcept { return m_desc.colorID; }
    RGResourceID GetDepthID() const noexcept { return m_desc.depthID; }

private:
    ViewTargetResourceDesc m_desc;
    ReleaseFn m_onRelease;
    bool m_ready{ false };
};