export module Runtime.Render.Text:GlyphTypes;

import Runtime.Render.Definition;

export struct GlyphMetrics
{
    // 렌더링 quad 크기(px). BitmapGlyph는 pixels.width/height와 항상 동일하지만,
    // MTSDFGlyph는 CreateGlyphInfo()에서 scale이 적용되어 pixels 크기와 달라질 수 있음.
    float width{ 0.0f };
    float height{ 0.0f };
    float pxRange{ 0.0f };

    float bearingX{ 0.0f };
    float bearingY{ 0.0f };
    float advanceX{ 0.0f };
};

export struct BitmapGlyph
{
    GlyphPixels pixels;
    GlyphMetrics metrics;
};

export struct MTSDFGlyph
{
    GlyphPixels pixels;
    GlyphMetrics metrics;
};