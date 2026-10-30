module;

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

export module DxRender.Resource:Font;

import std;
import :FreeTypeLibrary;
import Contract.Render.Assets;
import Contract.Render.IResource;

// 하나의 폰트 파일(.ttf)에 대응하는 런타임 폰트 객체.
// GPU 리소스가 아니므로 펜스로 Release할 필요가 없음. FreeType의 FT_Face 및 HarfBuzz 캐시 관리.
export class FontResource : public IResource
{
public:
    ~FontResource() override;
    FontResource() = default;

    bool IsReady() const noexcept override { return m_ready; }

    bool Initialize(FreeTypeLibrary& ftLibrary, std::shared_ptr<BinaryAsset> asset);

    FT_GlyphSlot GetGlyphSlot(uint32_t glyphIndex, uint32_t size) const;
    hb_font_t* GetHbFont(uint32_t size);
    FT_Face GetFtFace() const noexcept { return m_ftFace; }

    float GetLineHeight(uint32_t size) const;
    float GetAscent(uint32_t size) const;
    float GetUnderlineThickness(uint32_t size) const;
    float GetUnderlineOffset(uint32_t size) const;

private:
    struct SizeMetrics
    {
        float lineHeight;
        float ascent;
        float underlineThickness;
        float underlineOffset;
    };

    void SelectSize(uint32_t size) const; // face의 현재 픽셀 크기가 size가 아닐 때만 FT_Set_Pixel_Sizes 호출
    const SizeMetrics& MetricsFor(uint32_t size) const; // 크기별 메트릭을 최초 1회만 계산해 캐시
    static constexpr uint32_t kInvalidSize = std::numeric_limits<uint32_t>::max();

    FT_Face m_ftFace{ nullptr };
    std::unordered_map<uint32_t, hb_font_t*> m_hbFonts; // 크기별 HarfBuzz 폰트 캐시

    mutable uint32_t m_currentSize{ kInvalidSize }; // face에 마지막으로 설정된 픽셀 크기
    mutable std::unordered_map<uint32_t, SizeMetrics> m_metrics; // 노드 기반이라 rehash에도 참조 유지

    bool m_ready{ false };
    std::shared_ptr<BinaryAsset> m_asset;
};