export module Game.UserInterface.Widget:Image;

import :Widget;
import Contract.Render.UI;
import Core.Math;

export class Image final : public Widget
{
public:
    Image(BrushHandle bh, Core::Rect localRect)
        : Widget(localRect), m_bh(bh) {}

protected:
    void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const override
    {
        overlay.DrawImage(m_bh, screenRect);
    }

private:
    BrushHandle m_bh;
};