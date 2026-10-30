export module DxRender.Text:TextLayoutUtils;

import std;
import :TextTypes;
import DxRender.Definition;
import DxRender.Resource;
import Core.Math;
import Contract.Render.Backend.Types;

export float ComputeHorizontalOffset(
    TextHorizontalAlign align,
    float boundsWidth,
    float lineWidth)
{
    if (boundsWidth <= 0.f)
        return 0.f; // bounds width가 없으면 정렬 기준이 없으므로 Left와 동일

    switch (align)
    {
    case TextHorizontalAlign::Center: return (boundsWidth - lineWidth) * 0.5f;
    case TextHorizontalAlign::Right:  return (boundsWidth - lineWidth);
    case TextHorizontalAlign::Left:
    default: return 0.f;
    }
}

export float ComputeVerticalOffset(
    TextVerticalAlign align,
    float boundsHeight,
    float totalBlockHeight)
{
    if (boundsHeight <= 0.f)
        return 0.f; // bounds height가 없으면 정렬 기준이 없으므로 Top과 동일

    switch (align)
    {
    case TextVerticalAlign::Middle: return (boundsHeight - totalBlockHeight) * 0.5f;
    case TextVerticalAlign::Bottom: return (boundsHeight - totalBlockHeight);
    case TextVerticalAlign::Top:
    default: return 0.f;
    }
}

export std::vector<float> ComputeLineWidths(const std::vector<ShapedGlyph>& glyphs)
{
    std::vector<float> lineWidths;
    for (const auto& g : glyphs)
    {
        if (g.lineIndex >= lineWidths.size())
            lineWidths.resize(g.lineIndex + 1, 0.f);
        lineWidths[g.lineIndex] += g.advanceX;
    }
    return lineWidths;
}

export Core::Rect ComputeClipRect(const RenderTextItem& item)
{
	Core::Rect own{};
	if (item.layout.overflow == TextOverflow::Clip)
	{
		constexpr float kUnbounded = std::numeric_limits<float>::max(); // 해당 축 size가 지정 안 되어 있으면 그 축은 무제한
		own = Core::Rect{
			item.position.x,
			item.position.y,
			item.size.x > 0.f ? item.size.x : kUnbounded,
			item.size.y > 0.f ? item.size.y : kUnbounded
		};
	}

	if (!item.clip) // 바깥 클립(PushClip)이 없으면 자기 클립만.
		return own;

	if (!own.IsValid()) // 자기 클립이 없으면 바깥 클립만.
		return *item.clip;
	
    return own.Intersect(*item.clip); // 둘 다 있으면 교집합. 빈 교집합은 OverlayView::DrawText가 이미 걸러서 여기 오지 않는다.
}

export float ComputeLineHeight(const FontResource& font, std::uint32_t size, const TextLayout& layout)
{
    return font.GetLineHeight(size) * layout.lineSpacing;
}

// 너비 = 가장 긴 줄, 높이 = 줄 수 * 줄 높이
export Core::Vector2 ComputeBlockSize(std::span<const float> lineWidths, float lineHeight)
{
    float width = 0.f;
    for (float w : lineWidths)
        width = (std::max)(width, w);

    return { width, static_cast<float>(lineWidths.size()) * lineHeight };
}