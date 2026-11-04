module;

#include <Imgui/imgui.h>

export module UITool.PropertyPanel;

import std;
import Game.UserInterface;
import Core.Math;

// 선택한 위젯의 공통 레이아웃 속성(앵커, 피벗, 오프셋, 크기, 크기 정책, 툴팁)을 보여 주고 고치는 패널.
// 값을 바꿀 때는 위젯의 Set 함수만 쓰고, 읽을 때는 WidgetInspector를 쓴다.
export class PropertyPanel
{
public:
    void Draw(Widget* selected)
    {
        ImGui::Begin("Properties");
        if (!selected)
        {
            ImGui::TextUnformatted("선택한 위젯이 없습니다.");
            ImGui::End();
            return;
        }

        ImGui::TextUnformatted(WidgetInspector::TypeName(*selected).c_str());
        ImGui::Separator();

        DrawLayout(*selected);
        DrawTooltip(*selected);
        ImGui::End();
    }

private:
    static constexpr std::array<const char*, 3> kPolicyNames{ "Fixed", "Fit", "Fill" };

    static bool DrawPolicy(const char* label, SizePolicy& policy)
    {
        int index = static_cast<int>(policy);
        const bool changed = ImGui::Combo(label, &index, kPolicyNames.data(), static_cast<int>(kPolicyNames.size()));
        policy = static_cast<SizePolicy>(index);
        return changed;
    }

    void DrawLayout(Widget& widget)
    {
        Anchor anchor = WidgetInspector::GetAnchor(widget);
        bool anchorChanged = false;
        anchorChanged |= ImGui::DragFloat2("Anchor", &anchor.anchor.x, 0.01f, 0.f, 1.f, "%.2f");
        anchorChanged |= ImGui::DragFloat2("Pivot", &anchor.pivot.x, 0.01f, 0.f, 1.f, "%.2f");
        anchorChanged |= ImGui::DragFloat2("Offset", &anchor.offset.x, 1.f, 0.f, 0.f, "%.0f");
        if (anchorChanged)
            widget.SetAnchor(anchor);

        SizePolicy widthPolicy = WidgetInspector::WidthPolicy(widget);
        SizePolicy heightPolicy = WidgetInspector::HeightPolicy(widget);
        if (DrawPolicy("Width Policy", widthPolicy) | DrawPolicy("Height Policy", heightPolicy))
            widget.SetSizePolicy(widthPolicy, heightPolicy);

        // 크기는 정책이 Fixed인 축만 직접 정할 수 있다
        const Core::Rect rect = widget.LocalRect();
        float size[2]{ rect.width, rect.height };
        bool sizeChanged = false;
        ImGui::BeginDisabled(widthPolicy != SizePolicy::Fixed);
        sizeChanged |= ImGui::DragFloat("Width", &size[0], 1.f, 0.f, 0.f, "%.0f");
        ImGui::EndDisabled();
        ImGui::BeginDisabled(heightPolicy != SizePolicy::Fixed);
        sizeChanged |= ImGui::DragFloat("Height", &size[1], 1.f, 0.f, 0.f, "%.0f");
        ImGui::EndDisabled();
        if (sizeChanged)
            widget.SetSize(std::max(size[0], 0.f), std::max(size[1], 0.f));

        ImGui::Text("Local: %.0f, %.0f  %.0f x %.0f", rect.x, rect.y, rect.width, rect.height);
    }

    void DrawTooltip(Widget& widget)
    {
        // 입력 중에는 위젯 값으로 덮어쓰지 않는다. 선택이 바뀌었거나 입력 중이 아닐 때만 다시 채운다
        if (&widget != m_tooltipTarget || !m_tooltipEditing)
        {
            const std::string& current = WidgetInspector::Tooltip(widget);
            std::snprintf(m_tooltipBuffer.data(), m_tooltipBuffer.size(), "%s", current.c_str());
            m_tooltipTarget = &widget;
        }

        if (ImGui::InputText("Tooltip", m_tooltipBuffer.data(), m_tooltipBuffer.size()))
            widget.SetTooltip(m_tooltipBuffer.data());
        m_tooltipEditing = ImGui::IsItemActive();
    }

    std::array<char, 256> m_tooltipBuffer{};
    const Widget* m_tooltipTarget{ nullptr };
    bool m_tooltipEditing{ false };
};
