module Game.UserInterface.Widget;

import std;
import :Widget;

namespace
{
    bool Overlaps(const Core::Rect& a, const Core::Rect& b)
    {
        return a.x < b.x + b.width && b.x < a.x + a.width
            && a.y < b.y + b.height && b.y < a.y + a.height;
    }

    Core::Rect Intersection(const Core::Rect& a, const Core::Rect& b)
    {
        const float left = (std::max)(a.x, b.x);
        const float top = (std::max)(a.y, b.y);
        const float right = (std::min)(a.x + a.width, b.x + b.width);
        const float bottom = (std::min)(a.y + a.height, b.y + b.height);
        return { left, top, (std::max)(0.f, right - left), (std::max)(0.f, bottom - top) };
    }
}

Widget::Widget(Core::Rect localRect) // 처음 주어진 x, y는 TopLeft 앵커의 오프셋이 된다
    : m_rect(localRect)
    , m_anchor(Anchor::TopLeft().WithOffset(localRect.x, localRect.y))
{}

std::weak_ptr<Widget*> Widget::Handle()   // 호버/눌림 추적에 쓰일 때만 만든다.
{
    if (!m_handle)
        m_handle = std::make_shared<Widget*>(this);
    return m_handle;
}

// parentOrigin 기준으로 이 위젯의 화면 rect를 구한다. 그리기와 hit test가 공유하는 유일한 변환
Core::Rect Widget::ScreenRectUnder(const Core::Vector2& parentOrigin) const
{
    return { parentOrigin.x + m_rect.x, parentOrigin.y + m_rect.y, m_rect.width, m_rect.height };
}

Core::Vector2 Widget::ParentScreenOrigin() const
{
    Core::Vector2 o;
    for (const Widget* w = m_parent; w; w = w->m_parent)
        o = o + Core::Vector2{ w->m_rect.x, w->m_rect.y };
    return o;
}

Core::Rect Widget::ScreenRect() const
{
    return ScreenRectUnder(ParentScreenOrigin());
}

void Widget::Draw(IOverlayView& overlay) const
{
    DrawTree(overlay, ParentScreenOrigin(), std::nullopt);
}

void Widget::DrawTree(IOverlayView& overlay, const Core::Vector2& parentOrigin,
    const std::optional<Core::Rect>& clip) const
{
    const Core::Rect screen = ScreenRectUnder(parentOrigin);

    // 클립 영역 밖으로 완전히 밀려난 위젯은 그리기 명령을 만들지 않는다 (화면 밖으로 스크롤된 항목 비용 제거).
    // 전제: DrawSelf는 넘겨받은 screenRect 안에서만 그린다. 크기가 0인 위젯은 판정하지 않고 그린다.
    // 자손은 자기 위치로 따로 판정하므로, 부모가 가려져도 클립 안으로 들어온 자손은 그려진다.
    const bool empty = screen.width <= 0.f || screen.height <= 0.f;
    if (!clip || empty || Overlaps(screen, *clip))
        DrawSelf(overlay, screen);   // 자기 자신은 자기 클립 밖에서 그린다 (조상의 클립만 받는다)

    const bool clips = ClipsChildren();
    std::optional<Core::Rect> childClip = clip;
    if (clips)
    {
        overlay.PushClip(screen);
        childClip = clip ? Intersection(*clip, screen) : screen;   // 실제 교집합은 OverlayView가 맡고, 여기서는 건너뛰기 판정용으로만 계산
    }

    const Core::Vector2 childOrigin{ screen.x, screen.y };
    for (const auto& child : m_children)
        child->DrawTree(overlay, childOrigin, childClip);

    if (clips)
        overlay.PopClip();
}

Widget* Widget::Resolve(const Widget& root, const std::weak_ptr<Widget*>& handle)
{
    const auto token = handle.lock();
    Widget* w = token ? *token : nullptr;
    return (w && w->IsDescendantOf(root)) ? w : nullptr;   // 파괴됐으면 nullptr, Detach돼서 트리 밖이어도 nullptr
}