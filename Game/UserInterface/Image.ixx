export module Game.UserInterface:Image;

import :IWidget;
import Contract.Render.Handles;
import Contract.Render.IOverlayView;
import Core.Math;

export class Image final : public IWidget
{
public:
	Image(BrushHandle bh, Core::Rect dest) : m_bh(bh), m_dest(dest) {}

	void Draw(IOverlayView& overlay) const override
	{
		overlay.DrawUI(m_bh, m_dest);
	}

private:
	BrushHandle m_bh;
	Core::Rect  m_dest;
};