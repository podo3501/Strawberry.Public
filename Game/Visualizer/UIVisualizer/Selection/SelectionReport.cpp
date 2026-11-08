module UIVisualizer.SelectionReport;

import std;

namespace
{
    // from의 자손 중 target까지의 경로 조각들을 path에 채운다 (위에서 아래 순서). 찾으면 true
    bool FindPath(const Widget& from, const Widget& target, std::vector<std::string>& path)
    {
        const auto children = WidgetInspector::Children(from);
        for (std::size_t index = 0; index < children.size(); ++index)
        {
            const Widget& child = *children[index];
            const std::string& name = WidgetInspector::Name(child);
            path.push_back(name.empty() ? std::format("{}#{}", WidgetInspector::TypeName(child), index) : name);

            if (&child == &target || FindPath(child, target, path))
                return true;
            path.pop_back();
        }
        return false;
    }
}

std::string WidgetPath(const Widget& layerRoot, const Widget& widget)
{
    std::vector<std::string> path;
    if (!FindPath(layerRoot, widget, path))
        return {};

    std::string joined;
    for (const std::string& part : path)
    {
        if (!joined.empty())
            joined += '/';
        joined += part;
    }
    return joined;
}

std::string FormatSelection(const Widget& layerRoot, std::span<const Widget* const> items,
    std::string_view layoutFile, std::string_view time)
{
    std::string text = std::format("# UIVisualizer 선택 {}\nlayout: {}\n", time, layoutFile);
    if (items.empty())
        return text + "(선택 없음)\n";

    for (std::size_t i = 0; i < items.size(); ++i)
    {
        const Widget& widget = *items[i];
        const Core::Rect rect = WidgetInspector::ScreenRect(widget);
        const Anchor& anchor = WidgetInspector::GetAnchor(widget);
        text += std::format("{}\t{}\t{}\tscreen={},{},{}x{}\toffset={},{}\n",
            i + 1, WidgetPath(layerRoot, widget), WidgetInspector::TypeName(widget),
            rect.x, rect.y, rect.width, rect.height, anchor.offset.x, anchor.offset.y);
    }
    return text;
}
