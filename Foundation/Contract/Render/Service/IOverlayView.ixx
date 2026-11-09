export module Contract.Render.IOverlayView;

import Contract.Render.Handles;
import Contract.Render.Shared.Types;
import Core.Math;

export struct IOverlayView
{
	virtual ~IOverlayView() = default;

	virtual void DrawImage(
		BrushHandle bh,
		const Core::Rect& dest,
		const Core::Rect* source = nullptr, // source: 원본 이미지 안의 영역(픽셀 좌표, 0~1 비율 아님). nullptr이면 이미지 전체
		const Core::Color& tint = {}) = 0; // tint는 이미지 색에 곱해진다. RGB는 sRGB 값(눈에 보이는 색/255)이고 1을 넘으면 밝아지며, 알파는 전체 불투명도가 된다

	virtual void DrawText(
		FontHandle hF,
		TextRenderMode mode,
		std::string_view text,
		std::uint32_t size,
		const Core::Rect& bounds,
		const TextLayout& layout = {},
		const TextStyle& style = {}) = 0;

	virtual void DrawText(
		FontHandle hF,
		TextRenderMode mode,
		std::span<const TextSpan> spans,
		std::uint32_t size,
		const Core::Rect& bounds,
		const TextLayout& layout) = 0;

	virtual void PushClip(const Core::Rect& screenRect) = 0;
	virtual void PopClip() = 0;
};