export module UserInterface.Widget:Image;

import std;
import :Widget;
import :Slice;
import :SlicedBrush;
import :WidgetResources;
import Contract.Render.UI;
import Serialization;
import Core.Math;
import Core.TypeHierarchy;

export class Image final : public Core::TypeNode<Image, Widget>
{
public:
	Image(BrushHandle bh, Core::Rect localRect) // 일반 이미지: 슬라이스 없이 통째로 그림
		: TypeNode(localRect), m_brush(SlicedBrush::Plain(bh)) {}

	Image(BrushHandle bh, Core::Rect localRect, SliceMargins m, Core::Vector2 srcSize) // 슬라이스 이미지
		: TypeNode(localRect), m_brush(SlicedBrush::Sliced(bh, m, srcSize)) {}

protected:
	bool Serialize(Serializer& serializer, IWidgetResources& resources) override;   // 구현: Image.cpp

	void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const override
	{
		DrawSliced(overlay, m_brush, screenRect);
	}

private:
	SlicedBrush m_brush;
};
