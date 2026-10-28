export module Game.UserInterface.Widget:Image;

import :Widget;
import :SliceLayout;
import Contract.Render.UI;

export class Image final : public Widget
{
public:
	Image(BrushHandle bh, Core::Rect localRect) // 일반 이미지: 슬라이스 없이 통째로 그림
		: Widget(localRect), m_bh(bh) {}

	Image(BrushHandle bh, Core::Rect localRect, SliceMargins m, Core::Vector2 srcSize) // 슬라이스 이미지
		: Widget(localRect), m_bh(bh), m_margins(m), m_srcSize(srcSize) {}

protected:
	void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const override
	{
		if (!m_margins)
		{
			overlay.DrawImage(m_bh, screenRect);
			return;
		}

		for (const auto& q : BuildNineSlice(screenRect, *m_margins, m_srcSize))
			if (q.dest.width > 0.f && q.dest.height > 0.f)
				overlay.DrawImage(m_bh, q.dest, &q.uv);
	}

private:
	BrushHandle m_bh;
	std::optional<SliceMargins> m_margins;  // 없으면 일반 이미지
	Core::Vector2 m_srcSize{ 1.f, 1.f };
};