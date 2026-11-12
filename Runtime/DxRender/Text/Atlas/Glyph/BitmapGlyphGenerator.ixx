module;

#include <ft2build.h>
#include FT_FREETYPE_H

export module DxRender.Text:BitmapGlyphGenerator;

import std;
import :GlyphTypes;
import DxRender.Definition;
import DxRender.Resource;
import Core.Logger;

constexpr float FixedPointScale = 64.0f; // FreeType 26.6 고정소수점: 1.0 = 64

export BitmapGlyph GenerateBitmapGlyph(
    FontResource* font,
    std::uint32_t glyphIndex,
    std::uint32_t size)
{
    BitmapGlyph bitmap; // 만들지 못하면 빈 글리프(크기 0)를 돌려준다
    if (!font)
    {
        Core::Logger::Error("GenerateBitmapGlyph: 폰트가 없다 (glyphIndex={}, size={})", glyphIndex, size);
        return bitmap;
    }

    FT_GlyphSlot slot = font->GetGlyphSlot(glyphIndex, size); // 실패 원인은 GetGlyphSlot이 남긴다
    if (!slot)
        return bitmap;

    if (slot->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY) // 비트맵 전용이나 컬러 폰트는 GRAY가 아니다
    {
        Core::Logger::Error("GenerateBitmapGlyph: 지원하지 않는 픽셀 형식이다 (glyphIndex={}, size={}, pixel_mode={})",
            glyphIndex, size, static_cast<int>(slot->bitmap.pixel_mode));
        return bitmap;
    }

    auto& pixels = bitmap.pixels;
    auto& metrics = bitmap.metrics;

    pixels.format = GlyphFormat::R8;
    pixels.width = slot->bitmap.width;
    pixels.height = slot->bitmap.rows;

    metrics.width = static_cast<float>(slot->bitmap.width);
    metrics.height = static_cast<float>(slot->bitmap.rows);
    metrics.bearingX = static_cast<float>(slot->bitmap_left);
    metrics.bearingY = static_cast<float>(slot->bitmap_top);
    metrics.advanceX = slot->advance.x / FixedPointScale;

    if (pixels.width == 0 || pixels.height == 0)
        return bitmap;

    const std::size_t bytesPerPixel = GetBytesPerPixel(pixels.format); // R8: 1바이트/픽셀

    std::size_t pixelCount = static_cast<std::size_t>(pixels.width) * pixels.height * bytesPerPixel;
    pixels.buffer.resize(pixelCount);

    const int pitch = slot->bitmap.pitch;
    for (std::uint32_t y = 0; y < pixels.height; ++y)
    {
        const std::uint8_t* src = (pitch >= 0)
            ? slot->bitmap.buffer + y * pitch
            : slot->bitmap.buffer + (pixels.height - 1 - y) * (-pitch);

        std::memcpy(
            pixels.buffer.data() + y * pixels.width,
            src,
            pixels.width);
    }

    return bitmap;
}