module UIVisualizer.WidgetSelection;

import std;

namespace
{
    // widget의 자손 중 point에 있는 가장 위의 위젯. 자식이 부모보다 위이고, 나중 형제가 먼저 온 형제보다 위다. 없으면 nullptr
    const Widget* FindTopmost(const Widget& widget, const Core::Vector2& point)
    {
        const auto children = WidgetInspector::Children(widget);
        for (auto it = children.rbegin(); it != children.rend(); ++it)
        {
            const Widget& child = **it;
            if (const Widget* hit = FindTopmost(child, point))
                return hit;   // 자식이 부모의 영역 밖에 있어도 찾는다
            if (WidgetInspector::ScreenRect(child).Contains(point.x, point.y))
                return &child;
        }
        return nullptr;
    }
}

bool WidgetSelection::Toggle(const Widget& layerRoot, const Core::Vector2& point)
{
    const Widget* hit = FindTopmost(layerRoot, point);
    if (!hit)
        return false;

    if (const auto it = std::ranges::find(m_items, hit); it != m_items.end())
        m_items.erase(it);
    else
        m_items.push_back(hit);
    return true;
}
