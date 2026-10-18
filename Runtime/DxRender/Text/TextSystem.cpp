module DxRender.Text;

import :TextSystem;
import :TextLayout;
import :TextGeometry;
import DxRender.Definition;
import DxRender.Resource;
import Core.Assert;

TextSystem::TextSystem(
    Device& device,
    DescriptorFactory& factory,
    ResourceFactory& resFactory)
    : m_atlasBuilder{ resFactory }
    , m_fontAtlas{ device, factory, m_atlasBuilder }
{}

bool TextSystem::Initialize(const TextConfig& texConfig)
{
    return m_fontAtlas.Initialize(texConfig);
}

void TextSystem::AppendDrawItems(
    std::vector<RenderTextItem>& items,
    UIBatchBuffer& buffer)
{
    if (items.empty())
        return;

    auto shapedTexts = ShapeTexts(items);
    for (std::size_t i = 0; i < shapedTexts.size(); ++i)
        ApplyWordWrap(shapedTexts[i].glyphs, items[i].size.x, items[i].layout.wordWrap);

    m_fontAtlas.EnsureGlyphs(shapedTexts);

    for (const auto& shaped : shapedTexts)
    {
        if (shaped.glyphs.empty())
            continue;

        const auto& item = items[shaped.index];
        AppendShapedText(m_fontAtlas, shaped, item, buffer);
    }
}

std::vector<ShapedText> TextSystem::ShapeTexts(const std::vector<RenderTextItem>& items)
{
    std::vector<ShapedText> result;
    result.reserve(items.size());

    for (std::size_t index = 0; index < items.size(); ++index)
    {
        const auto& item = items[index];

        Core::Assert(item.fontRes);
        auto font = static_cast<FontResource*>(item.fontRes.get());

        ShapedText shaped;
        shaped.font = font;
        shaped.mode = item.mode;
        shaped.size = item.fontSize;
        shaped.index = index;
        shaped.glyphs = ShapeRuns(
            font,
            item.runs,
            item.fontSize);

        result.push_back(std::move(shaped));
    }

    return result;
}