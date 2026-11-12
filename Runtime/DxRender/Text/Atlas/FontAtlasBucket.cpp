module DxRender.Text;

import :FontAtlasBucket;
import Core.Assert;
import Core.Logger;

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
    m_atlasTextureSize = atlasTextureSize; // 크기가 올바른지는 FontAtlas::Initialize가 설정에서 검사한다
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

bool FontAtlasBucket::CreatePage()
{
    auto page = std::make_unique<AtlasPage>();

    if (!page->Initialize(
        m_device,
        m_factory,
        m_atlasTextureSize,
        GetAtlasPageFormat()))
    {
        Core::Logger::Error("FontAtlasBucket::CreatePage: 아틀라스 페이지를 만들지 못했다");
        return false;
    }

    m_pages.push_back(std::move(page));
    return true;
}

std::optional<FontAtlasBucket::GlyphPlacement> FontAtlasBucket::AllocateGlyphRect(
    const Core::Size& size,
    std::vector<std::vector<GlyphUploadEntry>>& outUploadsPerPage)
{
    if (size.width > m_atlasTextureSize.width || size.height > m_atlasTextureSize.height)
    {
        Core::Logger::Error("FontAtlasBucket::AllocateGlyphRect: 글리프({}x{})가 아틀라스 페이지({}x{})보다 크다",
            size.width, size.height, m_atlasTextureSize.width, m_atlasTextureSize.height);
        return std::nullopt;
    }

    if (m_pages.empty()) // 페이지가 클래스 생성 때는 없다.
    {
        if (!CreatePage())
            return std::nullopt;
        outUploadsPerPage.resize(m_pages.size());
    }

    auto pageIndex = CurrentPageIndex();
    auto pos = m_pages[pageIndex]->AllocateRect(size);
    if (!pos) // 현재 페이지가 가득 찼다.
    {
        if (!CreatePage())
            return std::nullopt;
        outUploadsPerPage.resize(m_pages.size());

        pageIndex = CurrentPageIndex();
        pos = m_pages[pageIndex]->AllocateRect(size);
        if (!pos)
        {
            Core::Logger::Error("FontAtlasBucket::AllocateGlyphRect: 새 페이지에도 글리프({}x{})가 들어가지 않는다", size.width, size.height);
            return std::nullopt;
        }
    }

    return GlyphPlacement{ pageIndex, *pos };
}

std::uint16_t FontAtlasBucket::CurrentPageIndex() const
{
    Core::Assert(!m_pages.empty());
    return static_cast<std::uint16_t>(m_pages.size() - 1);
}