module;

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

module DxRender.Resource;

import :Font;
import Core.Logger;

FontResource::~FontResource()
{
    for (auto& [size, hbFont] : m_hbFonts)
    {
        if (hbFont)
        {
            hb_font_destroy(hbFont);
        }
    }
    m_hbFonts.clear();
    m_metrics.clear();

    if (m_ftFace)
    {
        FT_Done_Face(m_ftFace);
        m_ftFace = nullptr;
    }
}

bool FontResource::Initialize(FreeTypeLibrary& ftLibrary, std::shared_ptr<BinaryAsset> asset)
{
    if (!asset || asset->buffer.empty())
    {
        Core::Logger::Error("FontResource::Initialize: 폰트 에셋이 없거나 비어 있다");
        return false;
    }

    FT_Error error = FT_New_Memory_Face(
        ftLibrary.Get(),
        reinterpret_cast<const FT_Byte*>(asset->buffer.data()),
        static_cast<FT_Long>(asset->buffer.size()),
        0, // 단일 페이스 인덱스
        &m_ftFace);

    if (error)
    {
        Core::Logger::Error("FontResource::Initialize: 폰트를 읽지 못했다 (FreeType 오류 코드={})", error);
        return false;
    }

    m_currentSize = kInvalidSize; // 새 face는 크기가 설정되지 않은 상태
    m_metrics.clear();

    m_asset = std::move(asset);
    m_ready = true;

    return true;
}

bool FontResource::SelectSize(uint32_t size) const
{
    if (size == 0) // FreeType은 0을 1로 올려 받지만 HarfBuzz 스케일은 0이 되어 어긋나므로 거부
    {
        Core::Logger::Error("FontResource::SelectSize: 글자 크기가 0이다");
        return false;
    }

    if (m_currentSize == size)
        return true;

    // 성공했을 때만 캐시를 갱신 (실패 시 face의 실제 크기와 어긋나지 않도록)
    const FT_Error error = FT_Set_Pixel_Sizes(m_ftFace, 0, size);
    if (error)
    {
        m_currentSize = kInvalidSize; // 실패하면 face의 크기를 알 수 없으므로 다음에 다시 맞춘다
        Core::Logger::Error("FontResource::SelectSize: 글자 크기를 정하지 못했다 (size={}, FreeType 오류 코드={})", size, error);
        return false;
    }

    m_currentSize = size;
    return true;
}

const FontResource::SizeMetrics& FontResource::MetricsFor(uint32_t size) const
{
    auto [it, inserted] = m_metrics.try_emplace(size);
    if (inserted)
    {
        if (!SelectSize(size))
        {
            static const SizeMetrics kEmpty{}; // 크기를 정하지 못하면 캐시에 남기지 않고 0을 돌려준다
            m_metrics.erase(it);
            return kEmpty;
        }

        const FT_Size_Metrics& m = m_ftFace->size->metrics;

        it->second.lineHeight = m.height / 64.0f;   // 26.6 fixed-point -> float
        it->second.ascent = m.ascender / 64.0f;
        it->second.underlineThickness = FT_MulFix(m_ftFace->underline_thickness, m.y_scale) / 64.0f;

        // underline_position은 폰트 디자인 좌표(+y 위쪽) 기준 baseline 대비 오프셋이라 보통 음수.
        // 화면 좌표(+y 아래쪽, baselineY에 더해 쓰는 값) 기준으로 바꾸려 부호를 뒤집는다.
        it->second.underlineOffset = -(FT_MulFix(m_ftFace->underline_position, m.y_scale) / 64.0f);
    }
    return it->second;
}

FT_GlyphSlot FontResource::GetGlyphSlot(uint32_t glyphIndex, uint32_t size) const
{
    if (!SelectSize(size))
        return nullptr;

    if (FT_Load_Glyph(m_ftFace, glyphIndex, FT_LOAD_DEFAULT))
        return nullptr;

    if (FT_Render_Glyph(m_ftFace->glyph, FT_RENDER_MODE_NORMAL))
        return nullptr;

    return m_ftFace->glyph;
}

hb_font_t* FontResource::GetHbFont(uint32_t size)
{
    // 캐시 적중 여부와 관계없이 반드시 호출. hb_shape가 face의 현재 크기를 읽을 수 있음.
    if (!SelectSize(size))
        return nullptr;

    auto it = m_hbFonts.find(size);
    if (it != m_hbFonts.end())
    {
        return it->second;
    }

    hb_font_t* hbFont = hb_ft_font_create_referenced(m_ftFace); // 해당 크기의 hb_font가 없으면 최초 1회만 생성
    if (hbFont)
    {
        hb_ft_font_set_funcs(hbFont);
        hb_font_set_scale(hbFont, size << 6, size << 6); // 생성 시점에 해당 크기로 스케일 고정
        m_hbFonts[size] = hbFont;
    }
    return hbFont;
}

float FontResource::GetLineHeight(uint32_t size) const { return MetricsFor(size).lineHeight; }
float FontResource::GetAscent(uint32_t size) const { return MetricsFor(size).ascent; }
float FontResource::GetUnderlineThickness(uint32_t size) const { return MetricsFor(size).underlineThickness; }
float FontResource::GetUnderlineOffset(uint32_t size) const { return MetricsFor(size).underlineOffset; }