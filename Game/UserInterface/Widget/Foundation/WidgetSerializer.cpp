module;

#include <nlohmann/json.hpp>

module UserInterface.Widget;

import std;
import :WidgetSerializer;
import :Widget;
import :Group;
import :Backdrop;
import :Button;
import :Dropdown;
import :Image;
import :Panel;
import :ProgressBar;
import :RadioGroup;
import :ScrollView;
import :Slider;
import :Text;
import :TextBox;
import :Toggle;
import :WidgetResources;
import :Stack;
import Serialization;
import Contract.Asset.Shared;
import Core.AssetPath;
import Core.TypeHierarchy;

using Json = nlohmann::json;

namespace
{
    using Creator = std::unique_ptr<Widget>(*)(const WidgetLoadContext&);

    struct Entry
    {
        std::string_view name;   // 파일에 적는 이름. 한 번 정하면 바꾸지 않는다
        Core::TypeID typeID;     // 정확한 클래스 (실행마다 달라질 수 있어 파일에는 적지 않는다)
        Creator creator;
    };

    // 저장을 지원하는 위젯 표: 이름 <-> 클래스(TypeID) <-> 생성 함수. 위젯을 늘릴 때 여기에 한 줄 더한다.
    // 표에 없는 클래스는 저장할 수 없다 (파생 클래스가 부모 이름으로 저장되는 일이 없다)
    const std::vector<Entry>& Entries()
    {
        static const std::vector<Entry> table{
            { "Dropdown", Core::GetTypeID<Dropdown>(), [](const WidgetLoadContext& context) -> std::unique_ptr<Widget> {
                return std::make_unique<Dropdown>(context.popupHost, Core::Rect{}, DropdownSkin{}, std::vector<std::string>{}); } },
            { "Group", Core::GetTypeID<Group>(), [](const WidgetLoadContext&) -> std::unique_ptr<Widget> { return std::make_unique<Group>(); } },
            { "Backdrop", Core::GetTypeID<Backdrop>(), [](const WidgetLoadContext&) -> std::unique_ptr<Widget> { return std::make_unique<Backdrop>(); } },
            { "Button", Core::GetTypeID<Button>(), [](const WidgetLoadContext&) -> std::unique_ptr<Widget> { return std::make_unique<Button>(Core::Rect{}); } },
            { "Image", Core::GetTypeID<Image>(), [](const WidgetLoadContext&) -> std::unique_ptr<Widget> { return std::make_unique<Image>(BrushHandle{}, Core::Rect{}); } },
            { "ProgressBar", Core::GetTypeID<ProgressBar>(), [](const WidgetLoadContext&) -> std::unique_ptr<Widget> { return std::make_unique<ProgressBar>(Core::Rect{}, GaugeSkin{}); } },
            { "ScrollView", Core::GetTypeID<ScrollView>(), [](const WidgetLoadContext&) -> std::unique_ptr<Widget> { return std::make_unique<ScrollView>(Core::Rect{}); } },
            { "RadioGroup", Core::GetTypeID<RadioGroup>(), [](const WidgetLoadContext&) -> std::unique_ptr<Widget> { return std::make_unique<RadioGroup>(StackAxis::Vertical, Core::Rect{}); } },
            { "Slider", Core::GetTypeID<Slider>(), [](const WidgetLoadContext&) -> std::unique_ptr<Widget> { return std::make_unique<Slider>(Core::Rect{}, SliderSkin{}); } },
            { "Text", Core::GetTypeID<Text>(), [](const WidgetLoadContext& context) -> std::unique_ptr<Widget> {
                return std::make_unique<Text>(context.measurer, FontHandle{}, TextRenderMode::MTSDF, std::string{}, 0u, Core::Rect{}); } },
            { "TextBox", Core::GetTypeID<TextBox>(), [](const WidgetLoadContext& context) -> std::unique_ptr<Widget> { return std::make_unique<TextBox>(context.measurer, Core::Rect{}, TextBoxSkin{}); } },
            { "Toggle", Core::GetTypeID<Toggle>(), [](const WidgetLoadContext&) -> std::unique_ptr<Widget> { return std::make_unique<Toggle>(Core::Rect{}, ToggleSkin{}); } },
            { "Panel", Core::GetTypeID<Panel>(), [](const WidgetLoadContext&) -> std::unique_ptr<Widget> { return std::make_unique<Panel>(StackAxis::Vertical, Core::Rect{}, SlicedBrush::Plain(BrushHandle{})); } },
            { "Stack", Core::GetTypeID<Stack>(), [](const WidgetLoadContext&) -> std::unique_ptr<Widget> { return std::make_unique<Stack>(StackAxis::Vertical, Core::Rect{}); } },
        };
        return table;
    }

    const Entry* FindByName(std::string_view name)
    {
        const auto& entries = Entries();
        const auto it = std::ranges::find(entries, name, &Entry::name);
        return it == entries.end() ? nullptr : &*it;
    }

    const Entry* FindByTypeID(Core::TypeID typeID)
    {
        const auto& entries = Entries();
        const auto it = std::ranges::find(entries, typeID, &Entry::typeID);
        return it == entries.end() ? nullptr : &*it;
    }
}

struct WidgetSerializer::Impl
{
    // 비어 있지 않은 이름이 다른 자식과 같은 자식이 있는가 (Attach는 겹침을 막지 못하므로 저장 때 확인한다)
    static bool HasDuplicateChildName(const Widget& widget)
    {
        std::unordered_set<std::string_view> names;
        for (const auto& child : widget.m_children)
            if (!child->m_name.empty() && !names.insert(child->m_name).second)
                return true;
        return false;
    }

    static bool Write(Widget& widget, IWidgetResources& resources, Json& outNode)
    {
        const Entry* entry = FindByTypeID(widget.GetTypeID());   // 정확한 클래스로 찾는다
        if (!entry || HasDuplicateChildName(widget))
            return false;

        outNode = Json::object();
        outNode["type"] = std::string(entry->name);
        auto serializer = Serializer::ForWrite(outNode);
        if (!widget.Serialize(serializer, resources) || serializer.HasError())   // 쓸 수 없는 값(이름 표 밖의 enum 등)이 있으면 저장은 거부된다
            return false;

        Json children = Json::array();
        for (const auto& child : widget.m_children)
        {
            Json childNode;
            if (!Write(*child, resources, childNode))
                return false;
            children.push_back(std::move(childNode));
        }
        outNode["children"] = std::move(children);
        return true;
    }

    static std::unique_ptr<Widget> Read(const Json& node, IWidgetResources& resources, const WidgetLoadContext& context)
    {
        if (!node.is_object() || !node.contains("type") || !node["type"].is_string())
            return nullptr;

        const Entry* entry = FindByName(node["type"].get<std::string>());
        if (!entry)
            return nullptr;

        auto widget = entry->creator(context);
        auto serializer = Serializer::ForRead(node);
        if (!widget->Serialize(serializer, resources) || serializer.HasError())   // 타입이 틀린 값이 있으면 불러오기 전체가 실패한다
            return nullptr;

        if (node.contains("children"))
        {
            if (!node["children"].is_array())
                return nullptr;
            for (const auto& childNode : node["children"])
            {
                auto child = Read(childNode, resources, context);
                if (!child)
                    return nullptr;
                widget->OnChildLoaded(widget->Attach(std::move(child)));
            }
            if (HasDuplicateChildName(*widget))   // 형제끼리 이름이 겹치는 파일은 통째로 실패
                return nullptr;
        }
        return widget;
    }
};

bool WidgetSerializer::Save(Widget& root, IAssetStorage& storage, IWidgetResources& resources, const Core::AssetPath& filename)
{
    Json rootNode;
    if (!Impl::Write(root, resources, rootNode))
        return false;

    Json file = Json::object();
    file["version"] = kVersion;
    file["root"] = std::move(rootNode);
    return storage.WriteText(filename, file.dump(2));
}

std::unique_ptr<Widget> WidgetSerializer::Load(IAssetStorage& storage, IWidgetResources& resources, const WidgetLoadContext& context, const Core::AssetPath& filename)
{
    std::string text;
    if (!storage.ReadText(filename, text))
        return nullptr;

    const Json file = Json::parse(text, nullptr, false);   // 실패해도 예외 대신 discarded
    if (file.is_discarded() || !file.is_object())
        return nullptr;
    if (!file.contains("version") || !file["version"].is_number_integer() || file["version"].get<int>() != kVersion)
        return nullptr;
    if (!file.contains("root"))
        return nullptr;

    return Impl::Read(file["root"], resources, context);
}
