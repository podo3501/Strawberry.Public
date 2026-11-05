module UserInterface.Widget;

import std;
import :Tooltip;
import :Widget;
import :LayoutTypes;

Core::Vector2 PlaceTooltip(Core::Vector2 pointer, Core::Vector2 size, Core::Vector2 screen, Core::Vector2 offset)
{
    const auto axis = [](float p, float extent, float limit, float gap)
        {
            float v = p + gap;
            if (v + extent > limit)
                v = p - gap - extent;
            return std::clamp(v, 0.f, (std::max)(0.f, limit - extent));
        };

    return { axis(pointer.x, size.x, screen.x, offset.x), axis(pointer.y, size.y, screen.y, offset.y) };
}

void TooltipController::Configure(TooltipFactory factory, TooltipSettings settings)
{
    Hide();
    m_factory = std::move(factory);
    m_settings = settings;
}

void TooltipController::Tick(float dt, Widget* hovered, Core::Vector2 pointer, Core::Vector2 screen, Widget& layer)
{
    Widget* source = FindSource(hovered);
    const std::weak_ptr<Widget*> handle = source ? source->Handle() : std::weak_ptr<Widget*>{};

    if (!SameWidget(handle, m_source))   // 대상이 바뀌었거나 사라짐: 숨기고 처음부터
    {
        Hide();
        m_source = handle;
        m_elapsed = 0.f;
        m_done = false;
    }

    if (!source || m_done)
        return;

    m_elapsed += dt;
    if (m_elapsed < m_settings.delay)
        return;

    m_done = true;   // 만들지 못했어도 이 대상에서는 다시 시도하지 않는다
    Show(*source, pointer, screen, layer);
}

void TooltipController::Dismiss()
{
    Hide();
    m_done = true;
}

Widget* TooltipController::FindSource(Widget* hovered)
{
    for (Widget* w = hovered; w; w = w->Parent())
        if (!w->Tooltip().empty())
            return w;
    return nullptr;
}

// 소유권을 쥐지 않고 비교해야 위젯이 죽었을 때 만료를 알 수 있다
bool TooltipController::SameWidget(const std::weak_ptr<Widget*>& a, const std::weak_ptr<Widget*>& b)
{
    return !a.owner_before(b) && !b.owner_before(a);
}

void TooltipController::Show(const Widget& source, Core::Vector2 pointer, Core::Vector2 screen, Widget& layer)
{
    if (!m_factory)
        return;

    std::unique_ptr<Widget> content = m_factory(source.Tooltip());
    if (!content)
        return;

    Widget& tip = layer.Attach(std::move(content));   // Fit 크기는 붙는 즉시 정해진다
    const Core::Rect& r = tip.LocalRect();
    const Core::Vector2 pos = PlaceTooltip(pointer, { r.width, r.height }, screen, m_settings.offset);
    tip.SetAnchor(Anchor::TopLeft().WithOffset(pos.x, pos.y));
    m_shown = &tip;
}

void TooltipController::Hide()
{
    if (!m_shown)
        return;

    m_shown->Detach();   // 반환값을 버려서 파괴
    m_shown = nullptr;
}