export module Service.Render:RenderView;

import std;
import Core.Math;
import Service.Render.Repository;

export enum class ViewType : std::uint8_t
{
    None,
    Scene,
    Overlay
};

export class RenderView
{
public:
    virtual ~RenderView() = default;

    RenderView(const RenderView&) = delete;
    RenderView& operator=(const RenderView&) = delete;
    RenderView(RenderView&&) = delete;
    RenderView& operator=(RenderView&&) = delete;

    virtual bool IsEmpty() const = 0;

    ViewType Type() const noexcept { return m_type; }

protected:
    RenderView(ViewType type, RepositoryContainer& repositories)
        : m_type{ type }
        , m_repositories{ repositories }
    {
    }

    ViewType m_type{ ViewType::None };
    RepositoryContainer& m_repositories;
};