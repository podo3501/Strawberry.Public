export module Runtime.Render.Definition:GlyphUploadData;

import std;
import :GlyphFormat;
import Runtime.Render.Core;

export struct GlyphUploadEntry // 하나의 글자를 아틀라스로 전송하기 위한 데이터 묶음
{
    // atlas 위치
    std::uint32_t x{ 0 };
    std::uint32_t y{ 0 };

    GlyphPixels pixels; // 업로드할 이미지 + glyph metric
};

export struct AtlasGlyphBatch
{
    const Resource* atlasResource{ nullptr };
    std::vector<GlyphUploadEntry> glyphs;
};