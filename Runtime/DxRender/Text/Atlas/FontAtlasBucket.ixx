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
import Core.Assert;

export class FontAtlasBucket
{
public:
    virtual ~FontAtlasBucket() = default;
    FontAtlasBucket() = delete;

    FontAtlasBucket(
        Device& device,
        DescriptorFactory& factory,
        FontBucketID bucketID) :
        m_device{ device },
        m_factory{ factory },
        m_bucketID{ bucketID }
    {
    }

    virtual void EnsureGlyphs(
        const ShapedText& shapedText,
        std::vector<std::vector<GlyphUploadEntry>>& outUploadsPerPage) = 0;

    void Initialize(const Core::Size& atlasTextureSize)
    {
        Core::Assert(atlasTextureSize.width > 0 && atlasTextureSize.height > 0);
        m_atlasTextureSize = atlasTextureSize;
    }

    const GlyphInfo* FindGlyph(
        FontResource* font,
        std::uint32_t glyphIndex,
        std::uint32_t size) const
    {
        return m_glyphCache.Get(
            font,
            glyphIndex,
            size);
    }

    std::shared_ptr<BrushResource> GetBrush(std::uint16_t pageIndex) const
    {
        Core::Assert(pageIndex < m_pages.size());
        return m_pages[pageIndex]->GetBrushResource();
    }

    const Resource& GetAtlasResource(std::uint16_t pageIndex) const
    {
        Core::Assert(pageIndex < m_pages.size());
        return m_pages[pageIndex]->GetAtlasResource();
    }

protected:
    void CreatePage()
    {
        auto page = std::make_unique<AtlasPage>();

        page->Initialize(
            m_device,
            m_factory,
            m_atlasTextureSize,
            GetAtlasPageFormat());

        m_pages.push_back(std::move(page));
    }

    std::uint16_t CurrentPageIndex() const
    {
        Core::Assert(!m_pages.empty());
        return static_cast<std::uint16_t>(m_pages.size() - 1);
    }

    virtual GlyphFormat GetAtlasPageFormat() const = 0;

protected:
    FontBucketID m_bucketID{ InvalidFontBucket };
    Core::Size m_atlasTextureSize{};
    GlyphCache m_glyphCache;
    std::vector<std::unique_ptr<AtlasPage>> m_pages;

private:
    Device& m_device;
    DescriptorFactory& m_factory;
};