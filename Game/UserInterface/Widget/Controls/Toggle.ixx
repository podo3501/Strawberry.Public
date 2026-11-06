export module UserInterface.Widget:Toggle;

import std;
import :Pressable;
import :WidgetResources;
import Contract.Render.IOverlayView;
import Serialization;
import Core.Math;
import Core.TypeHierarchy;

export struct ToggleSkin
{
    ButtonBrushes off;
    ButtonBrushes on;
};

// 체크박스/토글. 클릭할 때마다 켜짐/꺼짐이 바뀐다. 누른 채 바깥에서 떼면 바뀌지 않는다 (Button의 클릭 규칙과 같다).
// SetChecked는 콜백을 부르지 않고, 사용자 조작으로 값이 바뀔 때만 SetOnChanged 콜백이 불린다.
export class Toggle final : public Core::TypeNode<Toggle, Pressable>
{
public:
    Toggle(Core::Rect localRect, ToggleSkin skin, bool checked = false)
        : TypeNode(localRect), m_skin(skin), m_checked(checked) {}

    void SetChecked(bool checked) { m_checked = checked; }
    bool IsChecked() const { return m_checked; }
    void SetOnChanged(std::function<void(bool)> onChanged) { m_onChanged = std::move(onChanged); }

protected:
    bool Serialize(Serializer& serializer, IWidgetResources& resources) override;   // 구현: Toggle.cpp

    void OnClick(const PointerEvent&) override
    {
        m_checked = !m_checked;
        if (m_onChanged)
            m_onChanged(m_checked);   // 콜백이 위젯을 파괴할 수 있으니 마지막에 부른다
    }

    void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const override
    {
        DrawPicked(overlay, m_checked ? m_skin.on : m_skin.off, screenRect);
    }

private:
    ToggleSkin m_skin;
    bool m_checked;
    std::function<void(bool)> m_onChanged;
};