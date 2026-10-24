export module DxRender.Text:FontAtlas;

import std;
import :TextTypes;
import :AtlasPage;
import :FontSetting;
import :FontAtlasBucket;
import DxRender.Definition;
import DxRender.Resource;
import DxRender.Core;
import DxRender.Factory;
import Core.Utils;
import Contract.Render.Backend.Types;

struct FontBucketKey
{
    TextRenderMode mode{};
    FontBucketID bucket{ InvalidFontBucket };

    bool operator==(const FontBucketKey&) const = default;
};

struct FontBucketKeyHash
{
    std::size_t operator()(const FontBucketKey& key) const
    {
        return Core::HashOf(
            key.mode,
            key.bucket);
    }
};

export class FontAtlas
{
public:
    ~FontAtlas() = default;
    FontAtlas(Device& device, DescriptorFactory& factory);

    bool Initialize(const TextConfig& config);
    void EnsureGlyphs(const std::vector<ShapedText>& shapedTexts);

    [[nodiscard]] std::vector<AtlasGlyphBatch> TakePendingUploads();

    const GlyphInfo* FindGlyph(
        FontResource* font,
        TextRenderMode mode,
        std::uint32_t glyphIndex,
        std::uint32_t size) const;

    std::shared_ptr<BrushResource> GetBrush(const GlyphInfo* glyph) const;
    std::shared_ptr<BrushResource> GetSolidBrush() const;

private:
    void EnsureSolidPage();
    FontAtlasBucket* GetOrCreateBucket(const FontBucketKey& key);
    FontAtlasBucket* FindBucket(const FontBucketKey& key) const;
    const Resource& GetAtlasResource(const FontBucketKey& key, std::uint16_t pageIndex) const;

    Device& m_device;
    DescriptorFactory& m_factory;
    AtlasPage m_solidPage;

    TextConfig m_textConfig;
    std::unordered_map<FontBucketKey, std::unique_ptr<FontAtlasBucket>, FontBucketKeyHash> m_buckets;
    std::vector<AtlasGlyphBatch> m_pendingBatches;
};