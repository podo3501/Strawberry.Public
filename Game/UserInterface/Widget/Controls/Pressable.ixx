export module Game.UserInterface.Widget:Pressable;

import :Widget;
import :InputEvents;
import Contract.Input.Shared;
import Contract.Render.Handles;
import Core.Math;

export struct ButtonBrushes   // 순수 데이터: 클라이언트가 스킨을 만들 때 채우는 값
{
    BrushHandle normal{};
    BrushHandle hover{};
    BrushHandle pressed{};
};

// 누르면 캡처하고, hover/눌림/키보드 포커스를 추적하는 위젯의 공통 부모. Button, Toggle이 상속한다.
// 클릭 시 동작(OnClick)과 그리기(DrawSelf)는 자식이 정한다. Enter/Space는 클릭과 같은 동작을 한다.
class Pressable : public Widget
{
protected:
    explicit Pressable(Core::Rect localRect) : Widget(localRect) {}

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

    // 현재 상태에 맞는 브러시를 고른다
    BrushHandle Pick(const ButtonBrushes& brushes) const
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