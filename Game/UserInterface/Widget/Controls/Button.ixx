export module Game.UserInterface.Widget:Button;

import std;
import :Pressable;
import Contract.Render.IOverlayView;
import Core.Math;

export class Button : public Pressable
{
public:
    explicit Button(Core::Rect localRect, ButtonBrushes brushes = {})
        : Pressable(localRect), m_brushes(brushes) {}

    void SetOnClick(std::function<void()> onClick) { m_onClick = std::move(onClick); }

protected:
    void OnClick(const PointerEvent&) override { if (m_onClick) m_onClick(); }

    void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const override
    {
        overlay.DrawImage(Pick(m_brushes), screenRect);
    }

private:
    ButtonBrushes m_brushes;
    std::function<void()> m_onClick;
};