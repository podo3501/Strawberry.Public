module Game.UserInterface.Widget;

import std;
import :Widget;

// 기본 배치: 각 자식이 자기 앵커로 내 크기 안에서 위치를 정한다
void Widget::ArrangeChildren()
{
    const Core::Vector2 size = Size();
    for (auto& child : m_children)
        SetLocalPosition(*child, child->m_anchor.Resolve(size, child->Size()));
}

void Widget::SetLocalPosition(Widget& w, const Core::Vector2& pos)
{
    w.m_rect.x = pos.x;
    w.m_rect.y = pos.y;
}

Core::Vector2 Widget::MeasureContent() const
{
    return Size();
}

void Widget::MeasureTree()
{
    for (auto& child : m_children)
        child->MeasureTree();   // 자식 크기가 먼저 확정되어야 부모가 잴 수 있다

    if (!IsFit())
        return;                 // Fixed인 위젯은 내용 측정을 하지 않는다

    const Core::Vector2 content = MeasureContent();
    if (m_widthPolicy == SizePolicy::Fit)  m_rect.width = std::ceil(content.x);
    if (m_heightPolicy == SizePolicy::Fit) m_rect.height = std::ceil(content.y);
}

void Widget::ArrangeTree()
{
    ArrangeChildren();
    for (auto& child : m_children)
        child->ArrangeTree();
}

void Widget::InvalidateLayout()
{
    // Fit인 위젯은 크기가 내용에 달려 있으므로 그 부모도 다시 배치해야 한다
    Widget* top = this;
    while (top->m_parent && top->IsFit())
        top = top->m_parent;

    top->MeasureTree();
    top->ArrangeTree();
}

void Widget::Relayout()
{
    // 내 위치는 부모의 배치 방식이 정하고, 내 크기 변화는 형제 위치에도 영향을 줄 수 있다
    (m_parent ? m_parent : this)->InvalidateLayout();
}

void Widget::SetAnchor(const Anchor& anchor)
{
    m_anchor = anchor;
    Relayout();
}

void Widget::SetOffset(float x, float y)
{
    m_anchor.offset = { x, y };
    Relayout();
}

void Widget::SetSize(float width, float height)
{
    m_rect.width = width;
    m_rect.height = height;
    Relayout();   // 피벗이 0이 아니면 크기 변화가 위치도 바꾸므로 부모가 다시 배치
}

void Widget::SetSizePolicy(SizePolicy width, SizePolicy height)
{
    m_widthPolicy = width;
    m_heightPolicy = height;
    Relayout();
}