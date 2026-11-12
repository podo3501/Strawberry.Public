export module DxRender.Text:BitmapFontAtlasBucket;

import std;
import :FontAtlasBucket;
import :AtlasUtils;
import :GlyphTypes;
import :TextTypes;
import :BitmapGlyphGenerator;
import DxRender.Resource;
import DxRender.Definition;
import Core.Math;

export class BitmapFontAtlasBucket : public FontAtlasBucket
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

            BitmapGlyph bitmap = GenerateBitmapGlyph(
                font,
                glyphIndex,
                shapedText.size);

            if (bitmap.metrics.width == 0 || bitmap.metrics.height == 0)
            {
                m_glyphCache.Insert(
                    font,
                    glyphIndex,
                    shapedText.size,
                    CreateEmptyGlyphInfo(bitmap.metrics));

                continue;
            }

            Core::Size packSize{
                bitmap.pixels.width + Padding * 2,
                bitmap.pixels.height + Padding * 2 };

            auto placement = AllocateGlyphRect(packSize, outUploadsPerPage);
            if (!placement) // 자리를 못 잡은 글리프는 그리지 않고 다시 시도하지도 않는다.
            {
                m_glyphCache.Insert(font, glyphIndex, shapedText.size, GlyphInfo{});
                continue;
            }

            auto glyphInfo =
                CreateGlyphInfo(
                    bitmap,
                    m_bucketID,
                    placement->pageIndex,
                    placement->pos,
                    Padding,
                    m_atlasTextureSize);

            GlyphUploadEntry upload;
            if (CreateUploadEntry(
                std::move(bitmap.pixels),
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
        return GlyphFormat::R8;
    }
};