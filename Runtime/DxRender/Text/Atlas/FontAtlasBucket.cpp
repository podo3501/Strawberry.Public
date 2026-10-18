module DxRender.Text;

import :FontAtlasBucket;
import Core.Assert;

FontAtlasBucket::FontAtlasBucket(
    Device& device,
    DescriptorFactory& factory,
    FontBucketID bucketID) :
    m_device{ device },
    m_factory{ factory },
    m_bucketID{ bucketID }
{}

void FontAtlasBucket::Initialize(const Core::Size& atlasTextureSize)
{
    Core::Assert(atlasTextureSize.width > 0 && atlasTextureSize.height > 0);
    m_atlasTextureSize = atlasTextureSize;
}

const GlyphInfo* FontAtlasBucket::FindGlyph(
    FontResource* font,
    std::uint32_t glyphIndex,
    std::uint32_t size) const
{
    return m_glyphCache.Get(
        font,
        glyphIndex,
        size);
}

std::shared_ptr<BrushResource> FontAtlasBucket::GetBrush(std::uint16_t pageIndex) const
{
    Core::Assert(pageIndex < m_pages.size());
    return m_pages[pageIndex]->GetBrushResource();
}

const Resource& FontAtlasBucket::GetAtlasResource(std::uint16_t pageIndex) const
{
    Core::Assert(pageIndex < m_pages.size());
    return m_pages[pageIndex]->GetAtlasResource();
}

void FontAtlasBucket::CreatePage()
{
    auto page = std::make_unique<AtlasPage>();

    page->Initialize(
        m_device,
        m_factory,
        m_atlasTextureSize,
        GetAtlasPageFormat());

    m_pages.push_back(std::move(page));
}

std::uint16_t FontAtlasBucket::CurrentPageIndex() const
{
    Core::Assert(!m_pages.empty());
    return static_cast<std::uint16_t>(m_pages.size() - 1);
}