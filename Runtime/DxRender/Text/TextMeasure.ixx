export module DxRender.Text:TextMeasure;

import std;
import :TextTypes;
import :TextLayout;
import :TextLayoutUtils;
import DxRender.Resource;
import Contract.Render.Backend.Types;
import Core.Math;

export Core::Vector2 MeasureRuns(
    FontResource& font,
    const std::vector<TextRun>& runs,
    std::uint32_t size,
    float maxWidth,
    const TextLayout& layout)
{
    if (runs.empty())
        return {};

    auto glyphs = ShapeRuns(&font, runs, size);
    ApplyWordWrap(glyphs, maxWidth, layout.wordWrap);

    return ComputeBlockSize(
        ComputeLineWidths(glyphs),
        ComputeLineHeight(font, size, layout));
}