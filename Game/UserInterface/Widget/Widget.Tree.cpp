module Game.UserInterface.Widget;

import std;
import :Widget;
import Core.Assert;

void Widget::AttachImpl(std::unique_ptr<Widget> child, std::size_t index)
{
    Core::Assert(child); // Attach: null 위젯
    Core::Assert(!child->m_parent); // Attach: 이미 부모가 있음. Detach 먼저.
    Core::Assert(!IsDescendantOf(*child)); // Attach: 자기 자신/자손 밑에 붙이면 순환

    Widget& ref = *child;
    ref.m_parent = this;
    index = (std::min)(index, m_children.size());
    m_children.insert(m_children.begin() + static_cast<std::ptrdiff_t>(index),
        std::move(child));

    ref.Relayout();   // 붙는 즉시 현재 부모 크기 기준으로 배치 (형제 위치가 영향받는 컨테이너도 처리)
}

std::unique_ptr<Widget> Widget::Detach()
{
    Core::Assert(m_parent && "Detach: 부모가 없는 위젯");

    Widget* parent = m_parent;
    auto& siblings = parent->m_children;
    auto it = std::find_if(siblings.begin(), siblings.end(),
        [this](const std::unique_ptr<Widget>& p) { return p.get() == this; });
    Core::Assert(it != siblings.end());

    std::unique_ptr<Widget> self = std::move(*it);
    siblings.erase(it);
    m_parent = nullptr;

    parent->InvalidateLayout();   // 남은 형제 재배치 + 부모가 Fit이면 크기도 다시 계산
    return self;
}

bool Widget::IsDescendantOf(const Widget& ancestor) const
{
    for (const Widget* w = this; w; w = w->m_parent)
        if (w == &ancestor) return true;
    return false;
}