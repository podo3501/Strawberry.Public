module UIVisual.UiResources;

import std;
import Core.AssetPath;

template<typename Handle>
std::optional<std::string> UiResources::NameOf(const Entries<Handle>& entries, Handle handle)
{
    const auto it = std::ranges::find(entries, handle, &std::pair<std::string, Handle>::second);
    if (it == entries.end())
        return std::nullopt;
    return it->first;
}

// 이미 불러온 이름이면 그 핸들을, 아니면 불러와서 기억한다. 에셋 경로가 아닌 이름(확장자 없음 등)이나 불러오기에 실패한 이름은 nullopt
template<typename Handle, typename Loader>
std::optional<Handle> UiResources::FindOrLoad(Entries<Handle>& entries, std::string_view name, Loader load)
{
    const auto known = std::ranges::find(entries, name, &std::pair<std::string, Handle>::first);
    if (known != entries.end())
        return known->second;

    const auto path = Core::AssetPath::TryMake(name);
    if (!path || !path->HasExtension())   // ResourceID::MakePath는 확장자 없는 이름을 Assert로 막는다
        return std::nullopt;

    const Handle handle = load(Core::ResourceID::MakePath(*path));
    if (!handle)
        return std::nullopt;

    entries.emplace_back(std::string(name), handle);
    return handle;
}

std::optional<std::string> UiResources::BrushName(BrushHandle brush) const
{
    return NameOf(m_brushes, brush);
}

std::optional<std::string> UiResources::FontName(FontHandle font) const
{
    return NameOf(m_fonts, font);
}

std::optional<BrushHandle> UiResources::FindBrush(std::string_view name) const
{
    return FindOrLoad(m_brushes, name, [this](const Core::ResourceID& id) { return m_resources.LoadBrush(BrushDesc{ id }); });
}

std::optional<FontHandle> UiResources::FindFont(std::string_view name) const
{
    return FindOrLoad(m_fonts, name, [this](const Core::ResourceID& id) { return m_resources.LoadFont(FontDesc{ id }); });
}
