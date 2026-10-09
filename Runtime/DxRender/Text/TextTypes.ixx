export module DxRender.Text:TextTypes;

import std;
import :FontSetting;
import DxRender.Resource;
import Core.Math;
import Contract.Render.Definition;

export struct GlyphInfo
{
    TextRenderMode mode{ TextRenderMode::MTSDF };
    FontBucketID bucketID{ InvalidFontBucket };
    std::uint16_t pageIndex{ 0 };

    float width{ 0.0f };    // 그려질 글자의 크기. bitmap은 보여지는 것과 동일하지만, sdf는 거리장을 더한 만큼 크게 그린다. 즉 글자 사각형이 겹쳐지게 그려진다.
    float height{ 0.0f };
    float bearingX{ 0.0f }; // pen 위치에서 비트맵 왼쪽까지의 거리. pen은 현재 쓰여질 위치.
    float bearingY{ 0.0f }; // pen 위치에서 위쪽까지의 거리
    float pxRange{ 0.0f };  // range 값.

    Core::Vector2 uvMin{ 0.0f, 0.0f }; // Atlas Texture의 좌상단 UV. 즉 source
    Core::Vector2 uvMax{ 0.0f, 0.0f }; // Atlas Texture의 우하단 UV
    // ?!? advanceX는 어디? 이 값이 있어야 하는지 어떤지 나중에 판단 해 보자.
};

export struct ShapedGlyph
{
    std::uint32_t glyphIndex{ 0 };
    std::uint32_t sourceIndex{ 0 };
    std::uint32_t runIndex{ 0 };
    std::uint32_t lineIndex{ 0 };

    char32_t codepoint{ 0 };
    float advanceX{ 0.0f };
    float offsetX{ 0.0f };
    float offsetY{ 0.0f };
};

export struct ShapedText
{
    FontResource* font{ nullptr };
    TextRenderMode mode{ TextRenderMode::MTSDF };
    std::uint32_t size{ 0 };
    std::size_t index{ 0 }; // 원본 텍스트 위치 (여러 mode가 같이 그려지기 때문에)

    std::vector<ShapedGlyph> glyphs;
};