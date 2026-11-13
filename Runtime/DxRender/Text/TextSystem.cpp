module DxRender.Text;

import :TextSystem;
import :TextLayout;
import :TextGeometry;
import DxRender.Definition;
import DxRender.Resource;
import Core.Logger;
import Core.TypeHierarchy;

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

        // 호출하는 쪽이 위치로 짝을 맞추므로 항목마다 하나씩 만든다. 그릴 수 없는 항목은 글리프가 비어 있다.
        ShapedText shaped;
        shaped.mode = item.mode;
        shaped.size = item.fontSize;
        shaped.index = index;

        FontResource* font = Core::Cast<FontResource>(item.fontRes.get());
        if (!font)
        {
            Core::Logger::Error("TextSystem::ShapeTexts: 폰트가 없거나 폰트 리소스가 아닌 텍스트 항목이라 그리지 않는다 (index={})", index);
            result.push_back(std::move(shaped));
            continue;
        }

        shaped.font = font;
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