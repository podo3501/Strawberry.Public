export module Game.UserInterface.Widget:Image;

import :Widget;
import :Slice;
import :SlicedBrush;
import Contract.Render.UI;

export class Image final : public Widget
{
public:
	Image(BrushHandle bh, Core::Rect localRect) // 일반 이미지: 슬라이스 없이 통째로 그림
		: Widget(localRect), m_brush(SlicedBrush::Plain(bh)) {}

	Image(BrushHandle bh, Core::Rect localRect, SliceMargins m, Core::Vector2 srcSize) // 슬라이스 이미지
		: Widget(localRect), m_brush(SlicedBrush::Sliced(bh, m, srcSize)) {}

protected:
	void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const override
	{
		DrawSliced(overlay, m_brush, screenRect);
	}

private:
	SlicedBrush m_brush;
};