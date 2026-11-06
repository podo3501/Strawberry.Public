export module UserInterface.Widget:Pressable;

import :Widget;
import :InputEvents;
import :SlicedBrush;
import :WidgetResources;
import Contract.Input.Shared;
import Contract.Render.Handles;
import Contract.Render.IOverlayView;
import Serialization;
import Core.Math;
import Core.TypeHierarchy;

// 한 상태의 모습: 브러시와 그 위에 곱할 색. 틴트가 흰색(기본)이면 원본 그대로다. 1을 넘으면 밝아진다
export struct StateBrush
{
    BrushHandle brush{};
    Core::Color tint{};

    StateBrush() = default;
    StateBrush(BrushHandle brush, Core::Color tint = {}) : brush(brush), tint(tint) {}   // 브러시만 넣으면 틴트 없음
};

export struct ButtonBrushes   // 순수 데이터: 클라이언트가 스킨을 만들 때 채우는 값
{
    StateBrush normal{};
    StateBrush hover{};
    StateBrush pressed{};

    // 브러시 하나로 세 상태를 만들고 틴트만 다르게 한다. 기본값은 호버와 눌림이 차례로 조금씩 밝아진다
    static ButtonBrushes Tinted(BrushHandle brush, Core::Color hoverTint = { 1.15f, 1.15f, 1.15f }, Core::Color pressedTint = { 1.3f, 1.3f, 1.3f })
    {
        return { StateBrush{ brush }, StateBrush{ brush, hoverTint }, StateBrush{ brush, pressedTint } };
    }
};

// 저장/불러오기: 브러시 3개를 prefix가 붙은 키(normalBrush, hoverBrush, pressedBrush)로 적는다.
// 틴트는 흰색이 아닌 상태만 hasNormalTint + normalTintR/G/B/A 같은 키로 적고, 키가 없으면 흰색이다. 구현: Pressable.cpp
// 하나라도 이름을 모르는 핸들(저장), 이름이 없거나 모르는 이름(불러오기)이면 false.
export bool SerializeButtonBrushes(Serializer& serializer, IWidgetResources& resources, ButtonBrushes& brushes, std::string_view prefix);

// 누르면 캡처하고, hover/눌림/키보드 포커스를 추적하는 위젯의 공통 부모. Button, Toggle이 상속한다.
// 클릭 시 동작(OnClick)과 그리기(DrawSelf)는 자식이 정한다. Enter/Space는 클릭과 같은 동작을 한다.
class Pressable : public Core::TypeNode<Pressable, Widget>
{
protected:
    explicit Pressable(Core::Rect localRect) : TypeNode(localRect) {}

    bool IsHitTestable() const override { return true; }
    bool IsFocusable() const override { return true; }

    bool OnPointerDown(const PointerEvent&) override { m_down = true; return true; }
    void OnPointerUp(const PointerEvent&) override { m_down = false; }
    void OnPointerEnter() override { m_hover = true; }
    void OnPointerLeave() override { m_hover = false; }

    // 마우스로 받은 포커스는 표시하지 않는다. Tab으로 이동했을 때만 hover 모양으로 보여준다
    void OnFocus(bool viaKeyboard) override { m_focusVisible = viaKeyboard; }
    void OnBlur() override { m_focusVisible = false; }

    bool OnKeyDown(const KeyEvent& e) override
    {
        if (e.key != KeyCode::Enter && e.key != KeyCode::Space)
            return false;

        const Core::Rect r = ScreenRect();
        OnClick(PointerEvent{ Core::Vector2{ r.x + r.width * 0.5f, r.y + r.height * 0.5f } });   // 위젯 중심을 클릭한 것으로 취급
        return true;
    }

    // 현재 상태의 브러시를 그 상태의 틴트로 그린다
    void DrawPicked(IOverlayView& overlay, const ButtonBrushes& brushes, const Core::Rect& screenRect) const
    {
        const StateBrush& state = Pick(brushes);
        overlay.DrawImage(state.brush, screenRect, nullptr, state.tint);
    }

    // 현재 상태에 맞는 모습을 고른다
    const StateBrush& Pick(const ButtonBrushes& brushes) const
    {
        if (m_down && m_hover) return brushes.pressed;   // 누른 채 바깥으로 끌고 나가면 눌림 표시가 풀리도록 hover && down일 때만 Pressed
        if (m_hover || m_focusVisible) return brushes.hover;
        return brushes.normal;
    }

private:
    bool m_hover{ false };
    bool m_down{ false };
    bool m_focusVisible{ false };
};