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
		const Core::Rect* source = nullptr) = 0;

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