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

	void SetTint(const Core::Color& tint) { m_tint = tint; }   // 이미지에 곱하는 색(sRGB). 기본은 흰색(그대로). 흰색 이미지에 색을 입힐 때 쓴다

protected:
	bool Serialize(Serializer& serializer, IWidgetResources& resources) override;   // 구현: Image.cpp

	void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const override
	{
		DrawSliced(overlay, m_brush, screenRect, m_tint);
	}

private:
	SlicedBrush m_brush;
	Core::Color m_tint{};
};
