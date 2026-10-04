module;

#include <ft2build.h>
#include FT_FREETYPE_H

export module Runtime.Render.Text:BitmapGlyphGenerator;

import std;
import :GlyphTypes;
import Runtime.Render.Definition;
import Runtime.Render.Resource;
import Core.Assert;

constexpr float FixedPointScale = 64.0f; // FreeType 26.6 고정소수점: 1.0 = 64

export BitmapGlyph GenerateBitmapGlyph(
    FontResource* font,
    std::uint32_t glyphIndex,
    std::uint32_t size)
{
    Core::Assert(font);

    BitmapGlyph bitmap;
    FT_GlyphSlot slot = font->GetGlyphSlot(glyphIndex, size);
    if (!slot)
    {
        Core::Assert(false); // Failed to get glyph slot
        return bitmap; // 빈 BitmapGlyph 반환
    }
    Core::Assert(slot->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY);

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

    const std::size_t bytesPerPixel = GetBytesPerPixel(pixels.format);
    Core::Assert(bytesPerPixel == 1); // 이 함수는 R8(1바이트/픽셀) 포맷만 지원합니다.

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