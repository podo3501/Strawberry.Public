module;

#include <Imgui/imgui.h>

export module UITool.WidgetTreePanel;

import std;
import UserInterface;
import Core.Math;

// UserInterface의 위젯 트리를 보여 주고, 클릭한 위젯을 선택하는 패널.
export class WidgetTreePanel
{
public:
    // "Widgets" 창을 그린다. 선택은 이 안에서 갱신된다.
    void Draw(UserInterface& ui)
    {
        if (m_selected && !ContainsAnyLayer(ui, *m_selected))
            m_selected = nullptr;   // 선택한 위젯이 트리에서 사라졌다

        ImGui::Begin("Widgets");
        for (const LayerEntry& layer : kLayers)
            DrawNode(ui.Layer(layer.layer), layer.name);
        ImGui::End();
    }

    // 선택한 위젯의 테두리를 현재 ImGui 창(미리보기 패널)에 그린다. origin은 패널의 화면 좌표다.
    void DrawSelectionOutline(float originX, float originY) const
    {
        if (!m_selected) return;

        const Core::Rect rect = WidgetInspector::ScreenRect(*m_selected);
        const ImVec2 min{ originX + rect.x, originY + rect.y };
        const ImVec2 max{ min.x + rect.width, min.y + rect.height };
        ImGui::GetWindowDrawList()->AddRect(min, max, IM_COL32(255, 200, 0, 255), 0.f, 0, 2.f);
    }

    Widget* Selected() const { return m_selected; }

private:
    struct LayerEntry { UILayer layer; const char* name; };
    static constexpr std::array kLayers{
        LayerEntry{ UILayer::Hud, "Hud" },
        LayerEntry{ UILayer::Window, "Window" },
        LayerEntry{ UILayer::Popup, "Popup" },
        LayerEntry{ UILayer::Tooltip, "Tooltip" },
    };

    void DrawNode(Widget& widget, std::string_view layerName = {})
    {
        const auto children = WidgetInspector::Children(widget);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
        if (children.empty())
            flags |= ImGuiTreeNodeFlags_Leaf;
        if (&widget == m_selected)
            flags |= ImGuiTreeNodeFlags_Selected;

        const Core::Rect rect = WidgetInspector::ScreenRect(widget);
        const std::string label = layerName.empty()
            ? std::format("{}  ({:.0f}, {:.0f}  {:.0f}x{:.0f})", WidgetInspector::TypeName(widget), rect.x, rect.y, rect.width, rect.height)
            : std::string{ layerName };

        const bool open = ImGui::TreeNodeEx(&widget, flags, "%s", label.c_str());
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            m_selected = &widget;

        if (open)
        {
            for (const auto& child : children)
                DrawNode(*child);
            ImGui::TreePop();
        }
    }

    static bool Contains(const Widget& root, const Widget& target)
    {
        if (&root == &target) return true;

        for (const auto& child : WidgetInspector::Children(root))
            if (Contains(*child, target))
                return true;
        return false;
    }

    static bool ContainsAnyLayer(UserInterface& ui, const Widget& target)
    {
        return std::ranges::any_of(kLayers, [&](const LayerEntry& layer) { return Contains(ui.Layer(layer.layer), target); });
    }

    Widget* m_selected{ nullptr };
};
