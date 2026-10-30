export module DxRender.Text:TextLayout;

import std;
import :TextTypes;
import DxRender.Resource;
import Contract.Render.Backend.Types;

export void ApplyWordWrap(
    std::vector<ShapedGlyph>& glyphs,
    float maxWidth, 
    bool wordWrap);

export std::vector<ShapedGlyph> ShapeRuns(
    FontResource* font,
    const std::vector<TextRun>& runs,
    std::uint32_t fontSize);

export std::vector<ShapedGlyph> ShapeAndWrap(
    FontResource* font,
    const std::vector<TextRun>& runs,
    std::uint32_t size,
    float maxWidth,
    bool wordWrap);