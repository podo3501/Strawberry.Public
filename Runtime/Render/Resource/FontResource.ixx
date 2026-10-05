module;

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

export module Runtime.Render.Resource:Font;

import std;
import :FreeTypeLibrary;
import Client.Asset.Data;
import Client.Render.IResource;

// 하나의 폰트 파일(.ttf)에 대응하는 런타임 폰트 객체.
// GPU 리소스가 아니므로 펜스로 Release할 필요가 없음. FreeType의 FT_Face 및 HarfBuzz 캐시 관리.
export class FontResource : public IResource
{
public:
    virtual ~FontResource() override
    {
        for (auto& [size, hbFont] : m_hbFonts)
        {
            if (hbFont)
            {
                hb_font_destroy(hbFont);
            }
        }
        m_hbFonts.clear();

        if (m_ftFace)
        {
            FT_Done_Face(m_ftFace);
            m_ftFace = nullptr;
        }
    }

    FontResource() = default;

    virtual bool IsReady() const noexcept override { return m_ready; }

    bool Initialize(FreeTypeLibrary& ftLibrary, std::shared_ptr<BinaryAsset> asset)
    {
        FT_Error error = FT_New_Memory_Face(
            ftLibrary.Get(),
            reinterpret_cast<const FT_Byte*>(asset->buffer.data()),
            static_cast<FT_Long>(asset->buffer.size()),
            0, // 단일 페이스 인덱스
            &m_ftFace);

        if (error)
            return false;

        m_asset = std::move(asset);
        m_ready = true;

        return true;
    }

    FT_GlyphSlot GetGlyphSlot(uint32_t glyphIndex, uint32_t size) const
    {
        FT_Set_Pixel_Sizes(m_ftFace, 0, size);

        if (FT_Load_Glyph(m_ftFace, glyphIndex, FT_LOAD_DEFAULT))
            return nullptr;

        if (FT_Render_Glyph(m_ftFace->glyph, FT_RENDER_MODE_NORMAL))
            return nullptr;

        return m_ftFace->glyph;
    }

    hb_font_t* GetHbFont(uint32_t size)
    {
        FT_Set_Pixel_Sizes(m_ftFace, 0, size);

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

    FT_Face GetFtFace() const noexcept { return m_ftFace; }

    float GetLineHeight(uint32_t size) const
    {
        FT_Set_Pixel_Sizes(m_ftFace, 0, size);
        return m_ftFace->size->metrics.height / 64.0f; // 26.6 fixed-point -> float
    }

    float GetAscent(uint32_t size) const
    {
        FT_Set_Pixel_Sizes(m_ftFace, 0, size);
        return m_ftFace->size->metrics.ascender / 64.0f; // 26.6 fixed-point -> float
    }

    float GetUnderlineThickness(uint32_t size) const
    {
        FT_Set_Pixel_Sizes(m_ftFace, 0, size);
        FT_Fixed scaled = FT_MulFix(m_ftFace->underline_thickness, m_ftFace->size->metrics.y_scale);
        return scaled / 64.0f;
    }

    float GetUnderlineOffset(uint32_t size) const
    {
        FT_Set_Pixel_Sizes(m_ftFace, 0, size);
        // underline_position은 폰트 디자인 좌표(+y 위쪽) 기준 baseline 대비 오프셋이라 보통 음수.
        // 화면 좌표(+y 아래쪽, baselineY에 더해 쓰는 값) 기준으로 바꾸려 부호를 뒤집는다.
        FT_Fixed scaled = FT_MulFix(m_ftFace->underline_position, m_ftFace->size->metrics.y_scale);
        return -(scaled / 64.0f);
    }

private:
    FT_Face m_ftFace{ nullptr };
    std::unordered_map<uint32_t, hb_font_t*> m_hbFonts; // 크기별 HarfBuzz 폰트 캐시

    bool m_ready{ false };
    std::shared_ptr<BinaryAsset> m_asset;
};