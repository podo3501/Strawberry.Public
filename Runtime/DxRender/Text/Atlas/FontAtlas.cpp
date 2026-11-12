module DxRender.Text;

import :FontAtlas;
import :FontSetting;
import :BitmapFontAtlasBucket;
import :MTSDFFontAtlasBucket;
import Core.Math;
import Core.Assert;
import Core.Logger;

struct FontAtlasUpload
{
    std::vector<std::vector<GlyphUploadEntry>> pages;
};

FontAtlas::FontAtlas(Device& device, DescriptorFactory& factory)
    : m_device{ device }
    , m_factory{ factory }
{}

bool FontAtlas::Initialize(const TextConfig& config)
{
    for (const auto& [name, size] : { std::pair{ "bitmap", config.bitmap.atlasSize }, std::pair{ "mtsdf", config.mtsdf.atlasSize } })
    {
        if (size.width == 0 || size.height == 0)
        {
            Core::Logger::Error("FontAtlas::Initialize: {} 아틀라스 크기가 올바르지 않다 ({}x{})", name, size.width, size.height);
            return false;
        }
    }

    m_textConfig = config;
    return EnsureSolidPage();
}

void FontAtlas::EnsureGlyphs(const std::vector<ShapedText>& shapedTexts)
{
    std::unordered_map<FontBucketKey, FontAtlasUpload, FontBucketKeyHash> uploads;

    for (const auto& shapedText : shapedTexts)
    {
        const auto bucket = GetFontBucketID(shapedText.mode, shapedText.size);
        if (!bucket)
        {
            ReportNoBucket(shapedText.mode, shapedText.size);
            continue; // 버킷이 없는 크기의 텍스트는 그리지 않는다.
        }

        FontBucketKey key{ shapedText.mode, *bucket };
        auto& upload = uploads[key];

        auto atlasBucket = GetOrCreateBucket(key);
        if (!atlasBucket) // 모르는 렌더 모드는 Error가 이미 남았고 이 텍스트는 그리지 않는다.
            continue;

        atlasBucket->EnsureGlyphs(
            shapedText,
            upload.pages);
    }

    for (auto& [key, upload] : uploads)
    {
        for (std::uint16_t page = 0; page < upload.pages.size(); ++page)
        {
            auto& glyphs = upload.pages[page];
            if (glyphs.empty())
                continue;

            auto& atlasRes = GetAtlasResource(key, page);
            m_pendingBatches.push_back({ &atlasRes, std::move(glyphs) });
        }
    }
}

std::vector<AtlasGlyphBatch> FontAtlas::TakePendingUploads()
{
	return std::exchange(m_pendingBatches, {});
}

const GlyphInfo* FontAtlas::FindGlyph(
    FontResource* font,
    TextRenderMode mode,
    std::uint32_t glyphIndex,
    std::uint32_t size) const
{
    const auto bucket = GetFontBucketID(mode, size);
    if (!bucket)
        return nullptr;

    FontBucketKey key{ mode, *bucket };
    auto atlasBucket = FindBucket(key);
    if (!atlasBucket)
        return nullptr;

    return atlasBucket->FindGlyph(
        font,
        glyphIndex,
        size);
}

std::shared_ptr<BrushResource> FontAtlas::GetBrush(const GlyphInfo* glyph) const
{
    if (!glyph) return nullptr;

    FontBucketKey key{ glyph->mode, glyph->bucketID };
    auto atlasBucket = FindBucket(key);
    Core::Assert(atlasBucket);

    return atlasBucket->GetBrush(glyph->pageIndex);
}

std::shared_ptr<BrushResource> FontAtlas::GetSolidBrush() const
{
    return m_solidPage.GetBrushResource();
}

void FontAtlas::ReportNoBucket(TextRenderMode mode, std::uint32_t size)
{
    // 매 프레임 불리므로 같은 모드와 크기는 한 번만 남긴다.
    const auto key = (static_cast<std::uint64_t>(mode) << 32) | size;
    if (!m_reportedNoBucket.insert(key).second)
        return;

    Core::Logger::Error("FontAtlas::EnsureGlyphs: 글자 크기 {}에 맞는 아틀라스 버킷이 없어 그리지 않는다 (mode={})", size, static_cast<int>(mode));
}

bool FontAtlas::EnsureSolidPage()
{
    constexpr Core::Size SolidPageSize{ 4, 4 };
    if (!m_solidPage.Initialize(m_device, m_factory, SolidPageSize, GlyphFormat::RGBA8)) // 알파 포함 RGBA 필요
    {
        Core::Logger::Error("FontAtlas::EnsureSolidPage: 단색 페이지를 만들지 못했다");
        return false;
    }

    // 4x4를 전부 흰색(premultiplied RGBA = 255,255,255,255)으로 채워 1회 업로드
    auto format = GlyphFormat::RGBA8;
    GlyphPixels pixels{
        .format = format,
        .width = SolidPageSize.width,
        .height = SolidPageSize.height
    };

    pixels.buffer.resize(pixels.width * pixels.height * GetBytesPerPixel(format));
    std::fill(pixels.buffer.begin(), pixels.buffer.end(), std::uint8_t{ 0xFF });

    GlyphUploadEntry entry;
    entry.x = 0;
    entry.y = 0;
    entry.pixels = std::move(pixels);

    std::vector<GlyphUploadEntry> uploads;
    uploads.push_back(std::move(entry));

    m_pendingBatches.push_back({ &m_solidPage.GetAtlasResource(), std::move(uploads) });
    return true;
}

FontAtlasBucket* FontAtlas::GetOrCreateBucket(const FontBucketKey& key)
{
    auto iter = m_buckets.find(key);
    if (iter != m_buckets.end())
        return iter->second.get();

    Core::Size atlasSize{};
    std::unique_ptr<FontAtlasBucket> fontAtlasBucket{ nullptr };
    switch (key.mode)
    {
    case TextRenderMode::Bitmap:
        fontAtlasBucket = std::make_unique<BitmapFontAtlasBucket>(m_device, m_factory, key.bucket);
        atlasSize = m_textConfig.bitmap.atlasSize;
        break;
    case TextRenderMode::MTSDF:
        fontAtlasBucket = std::make_unique<MTSDFFontAtlasBucket>(m_device, m_factory, key.bucket);
        atlasSize = m_textConfig.mtsdf.atlasSize;
        break;
    default:
        Core::Logger::Error("FontAtlas::GetOrCreateBucket: 알 수 없는 TextRenderMode ({})", static_cast<int>(key.mode));
        return nullptr;
    }
    fontAtlasBucket->Initialize(atlasSize);

    auto result = fontAtlasBucket.get();
    m_buckets.emplace(key, std::move(fontAtlasBucket));

    return result;
}

FontAtlasBucket* FontAtlas::FindBucket(const FontBucketKey& key) const
{
    auto iter = m_buckets.find(key);
    if (iter == m_buckets.end())
        return nullptr;

    return iter->second.get();
}

const Resource& FontAtlas::GetAtlasResource(const FontBucketKey& key, std::uint16_t pageIndex) const
{
    auto atlasBucket = FindBucket(key);
    Core::Assert(atlasBucket);

    return atlasBucket->GetAtlasResource(pageIndex);
}
