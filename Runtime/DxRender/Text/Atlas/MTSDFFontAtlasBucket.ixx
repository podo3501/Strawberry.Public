export module DxRender.Text:MTSDFFontAtlasBucket;

import std;
import Core.Math;
import DxRender.Definition;
import :FontAtlasBucket;
import :AtlasUtils;
import :TextTypes;
import DxRender.Resource;
import :MTSDFGlyphGenerator;

export class MTSDFFontAtlasBucket : public FontAtlasBucket
{
public:
    using FontAtlasBucket::FontAtlasBucket;

    void EnsureGlyphs(
        const ShapedText& shapedText,
        std::vector<std::vector<GlyphUploadEntry>>& outUploadsPerPage) override
    {
        constexpr std::uint32_t Padding = 1;

        outUploadsPerPage.resize(m_pages.size());
        auto font = shapedText.font;
        for (const auto& shapedGlyph : shapedText.glyphs)
        {
            std::uint32_t glyphIndex = shapedGlyph.glyphIndex;
            if (m_glyphCache.Get(font, glyphIndex, shapedText.size))
                continue;

            MTSDFGlyph mtsdf = GenerateMTSDFGlyph(
                font,
                glyphIndex,
                shapedText.size);
            auto& pixels = mtsdf.pixels;
            auto& metrics = mtsdf.metrics;

            if (metrics.width == 0.f || metrics.height == 0.f)
            {
                m_glyphCache.Insert(
                    font,
                    glyphIndex,
                    shapedText.size,
                    CreateEmptyGlyphInfo(metrics));

                continue;
            }

            Core::Size packSize{
                pixels.width + Padding * 2,
                pixels.height + Padding * 2 };

            auto placement = AllocateGlyphRect(packSize, outUploadsPerPage);
            if (!placement) // 자리를 못 잡은 글리프는 그리지 않고 다시 시도하지도 않는다.
            {
                m_glyphCache.Insert(font, glyphIndex, shapedText.size, GlyphInfo{});
                continue;
            }

            std::uint32_t bakeSize = GetMTSDFResolution(m_bucketID); // 이 버킷이 실제로 굽는 해상도
            float scale = static_cast<float>(shapedText.size) / static_cast<float>(bakeSize);

            auto glyphInfo =
                CreateGlyphInfo(
                    mtsdf,
                    m_bucketID,
                    placement->pageIndex,
                    placement->pos,
                    Padding,
                    m_atlasTextureSize,
                    scale);

            GlyphUploadEntry upload;
            if (CreateUploadEntry(
                std::move(mtsdf.pixels),
                placement->pos,
                Padding,
                upload))
            {
                outUploadsPerPage[placement->pageIndex].push_back(
                    std::move(upload));
            }

            m_glyphCache.Insert(
                font,
                glyphIndex,
                shapedText.size,
                glyphInfo);
        }
    }

private:
    GlyphFormat GetAtlasPageFormat() const override
    {
        return GlyphFormat::RGBA8;
    }
};