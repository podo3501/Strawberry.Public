module DxRender.Text;

import :TextSystem;
import :TextLayout;
import :TextGeometry;
import DxRender.Definition;
import DxRender.Resource;
import Core.Assert;

TextSystem::TextSystem(Device& device, DescriptorFactory& factory)
    : m_fontAtlas{ device, factory }
{}

bool TextSystem::Initialize(const TextConfig& texConfig)
{
    return m_fontAtlas.Initialize(texConfig);
}

std::vector<ShapedText> TextSystem::PrepareDrawItems(const std::vector<RenderTextItem>& items)
{
    if (items.empty())
        return {};

    auto shapedTexts = ShapeTexts(items);
    m_fontAtlas.EnsureGlyphs(shapedTexts);
    return shapedTexts;
}

void TextSystem::AppendDrawItem(const ShapedText& shaped, const RenderTextItem& item, UIBatchBuffer& buffer)
{
    if (shaped.glyphs.empty())
        return;

    AppendShapedText(m_fontAtlas, shaped, item, buffer);
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
        shaped.glyphs = ShapeAndWrap(
            font,
            item.runs,
            item.fontSize,
            item.size.x,
            item.layout.wordWrap);

        result.push_back(std::move(shaped));
    }

    return result;
}