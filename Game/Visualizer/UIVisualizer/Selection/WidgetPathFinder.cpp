module UIVisualizer.WidgetPathFinder;

import std;

namespace
{
    // "종류#순서" 조각이 가리키는 자식. 형식이 아니거나 그 자리에 맞는 이름 없는 자식이 없으면 nullptr
    const Widget* FindUnnamedChild(const Widget& parent, std::string_view segment)
    {
        const auto hash = segment.rfind('#');
        if (hash == std::string_view::npos)
            return nullptr;

        const std::string_view type = segment.substr(0, hash);
        const std::string_view number = segment.substr(hash + 1);
        std::size_t index = 0;
        const auto [end, error] = std::from_chars(number.data(), number.data() + number.size(), index);
        if (error != std::errc{} || end != number.data() + number.size())
            return nullptr;

        const auto children = WidgetInspector::Children(parent);
        if (index >= children.size())
            return nullptr;

        const Widget& child = *children[index];
        if (!WidgetInspector::Name(child).empty() || WidgetInspector::TypeName(child) != type)
            return nullptr;
        return &child;
    }

    const Widget* FindNamedChild(const Widget& parent, std::string_view name)
    {
        for (const auto& child : WidgetInspector::Children(parent))
            if (WidgetInspector::Name(*child) == name)
                return child.get();
        return nullptr;
    }
}

const Widget* FindWidgetByPath(const Widget& layerRoot, std::string_view path)
{
    if (path.empty())
        return nullptr;

    const Widget* current = &layerRoot;
    while (current)
    {
        const auto slash = path.find('/');
        const std::string_view segment = path.substr(0, slash);
        if (segment.empty())
            return nullptr;

        // 이름에는 '#'을 쓸 수 없으므로 '#'이 있는 조각은 이름 없는 위젯의 임시 표기다
        current = segment.contains('#') ? FindUnnamedChild(*current, segment) : FindNamedChild(*current, segment);
        if (slash == std::string_view::npos)
            return current;
        path = path.substr(slash + 1);
    }
    return nullptr;
}

Widget* FindWidgetByPath(Widget& layerRoot, std::string_view path)
{
    return const_cast<Widget*>(FindWidgetByPath(std::as_const(layerRoot), path));   // 바꿀 수 있는 트리에서 찾은 위젯이므로 const를 되돌려도 안전하다
}
