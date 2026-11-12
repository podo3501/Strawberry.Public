export module DxRender.Text:FontAtlasBucket;

import std;
import :FontSetting;
import :AtlasPage;
import :GlyphCache;
import :TextTypes;
import DxRender.Core;
import DxRender.Factory;
import DxRender.Definition;
import DxRender.Resource;
import Core.Math;

export class FontAtlasBucket
{
public:
    virtual ~FontAtlasBucket() = default;
    FontAtlasBucket() = delete;
    FontAtlasBucket(
        Device& device,
        DescriptorFactory& factory,
        FontBucketID bucketID);

    virtual void EnsureGlyphs(
        const ShapedText& shapedText,
        std::vector<std::vector<GlyphUploadEntry>>& outUploadsPerPage) = 0;

    void Initialize(const Core::Size& atlasTextureSize);
    const GlyphInfo* FindGlyph(
        FontResource* font,
        std::uint32_t glyphIndex,
        std::uint32_t size) const;

    std::shared_ptr<BrushResource> GetBrush(std::uint16_t pageIndex) const;
    const Resource& GetAtlasResource(std::uint16_t pageIndex) const;

protected:
    struct GlyphPlacement
    {
        std::uint16_t pageIndex{ 0 };
        Core::Point pos{};
    };

    // 현재 페이지에 자리를 잡고, 가득 차면 새 페이지에 잡는다. 실패하면 Error를 남기고 nullopt이며 outUploadsPerPage는 페이지 수에 맞춘다.
    std::optional<GlyphPlacement> AllocateGlyphRect(
        const Core::Size& size,
        std::vector<std::vector<GlyphUploadEntry>>& outUploadsPerPage);

    bool CreatePage();
    std::uint16_t CurrentPageIndex() const;
    virtual GlyphFormat GetAtlasPageFormat() const = 0;

    FontBucketID m_bucketID{ InvalidFontBucket };
    Core::Size m_atlasTextureSize{};
    GlyphCache m_glyphCache;
    std::vector<std::unique_ptr<AtlasPage>> m_pages;

private:
    Device& m_device;
    DescriptorFactory& m_factory;
};