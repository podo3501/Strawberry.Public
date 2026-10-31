export module Game.UserInterface.Widget:Button;

import std;
import :Widget;
import Contract.Render.Handles;
import Contract.Render.IOverlayView;
import Core.Math;

export struct ButtonBrushes
{
    BrushHandle normal{};
    BrushHandle hover{};
    BrushHandle pressed{};
};

export class Button : public Widget
{
public:
    explicit Button(Core::Rect localRect, ButtonBrushes brushes = {})
        : Widget(localRect), m_brushes(brushes) {}

    void SetOnClick(std::function<void()> onClick) { m_onClick = std::move(onClick); }

protected:
    bool IsHitTestable() const override { return true; }
    bool OnPointerDown(const PointerEvent&) override { m_down = true; return true; }
    void OnPointerUp(const PointerEvent&) override { m_down = false; }
    void OnClick(const PointerEvent&) override { if (m_onClick) m_onClick(); }
    void OnPointerEnter() override { m_hover = true; }
    void OnPointerLeave() override { m_hover = false; }

    void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const override
    {
        overlay.DrawImage(CurrentBrush(), screenRect);
    }

private:
    BrushHandle CurrentBrush() const
    {
        if (m_down && m_hover) return m_brushes.pressed; // 누른 채 바깥으로 끌고 나가면 눌림 표시가 풀리도록 hover && down일 때만 Pressed
        if (m_hover) return m_brushes.hover;
        return m_brushes.normal;
    }

    ButtonBrushes m_brushes;
    std::function<void()> m_onClick;
    bool m_hover{ false };
    bool m_down{ false };
};