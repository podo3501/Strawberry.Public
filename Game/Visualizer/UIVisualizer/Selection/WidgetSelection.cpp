module UIVisualizer.WidgetSelection;

import std;

namespace
{
    // widget의 자손 중 point에 있는 가장 위의 위젯. 자식이 부모보다 위이고, 나중 형제가 먼저 온 형제보다 위다. 없으면 nullptr
    Widget* FindTopmost(const Widget& widget, const Core::Vector2& point)
    {
        const auto children = WidgetInspector::Children(widget);
        for (auto it = children.rbegin(); it != children.rend(); ++it)
        {
            Widget& child = **it;
            if (Widget* hit = FindTopmost(child, point))
                return hit;   // 자식이 부모의 영역 밖에 있어도 찾는다
            if (WidgetInspector::ScreenRect(child).Contains(point.x, point.y))
                return &child;
        }
        return nullptr;
    }

    // from의 자손 중 target까지의 경로(from의 자식부터 target까지, 위에서 아래 순서)를 path에 채운다. 찾으면 true
    bool FindPathTo(const Widget& from, const Widget* target, std::vector<Widget*>& path)
    {
        for (const auto& child : WidgetInspector::Children(from))
        {
            path.push_back(child.get());
            if (child.get() == target || FindPathTo(*child, target, path))
                return true;
            path.pop_back();
        }
        return false;
    }

    // point에 보이는 가장 위의 위젯부터 레이어 루트 바로 아래의 조상까지 (안쪽부터). 위젯이 없으면 빈 체인
    std::vector<Widget*> ChainAt(Widget& layerRoot, const Core::Vector2& point)
    {
        std::vector<Widget*> chain;
        if (Widget* hit = FindTopmost(layerRoot, point))
        {
            FindPathTo(layerRoot, hit, chain);
            std::ranges::reverse(chain);
        }
        return chain;
    }
}

bool WidgetSelection::Pick(Widget& layerRoot, const Core::Vector2& point)
{
    std::vector<Widget*> chain = ChainAt(layerRoot, point);
    if (chain.empty())
        return false;

    // 가장 안쪽 위젯이 그대로면 같은 곳을 다시 클릭한 것이므로 한 칸 올라가고, 다르면 새로 시작한다
    const bool continuing = HasPick() && chain.front() == m_chain.front();
    if (!continuing)
        m_anchor = point;   // 새로 시작할 때만 클릭 위치를 기준으로 삼는다

    m_pickIndex = continuing ? (m_pickIndex + 1) % chain.size() : 0;
    m_chain = std::move(chain);
    return true;
}

std::span<Widget* const> WidgetSelection::PickWindow() const noexcept
{
    const std::size_t count = m_chain.size();
    if (count <= kPickWindowSize)
        return m_chain;

    const std::size_t half = kPickWindowSize / 2;
    const std::size_t start = std::min(m_pickIndex > half ? m_pickIndex - half : 0, count - kPickWindowSize);
    return std::span{ m_chain }.subspan(start, kPickWindowSize);
}

bool WidgetSelection::SelectPicked()
{
    if (!HasPick())
        return false;

    Widget* picked = m_chain[m_pickIndex];
    CancelPick();

    if (const auto it = std::ranges::find(m_items, picked); it != m_items.end())
        m_items.erase(it);
    else
        m_items.push_back(picked);
    return true;
}
