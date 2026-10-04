export module Runtime.Render.Text:TextSystem;

import std;
import :TextTypes;
import :TextLayout;
import :TextGeometry;
import :FontAtlas;
import Runtime.Render.Definition;
import Runtime.Render.Resource;
import Runtime.Render.Core;
import Runtime.Render.Factory;
import Pipeline.GraphBuilder;
import Core.Assert;
import Client.Render.Definition;

export class TextSystem
{
public:
    ~TextSystem() = default;

    TextSystem(
        Device& device,
        DescriptorFactory& factory,
        ResourceFactory& resFactory)
        : m_atlasBuilder{ resFactory }
        , m_fontAtlas{ device, factory, m_atlasBuilder }
    {
    }

    bool Initialize(const TextConfig& texConfig)
    {
        return m_fontAtlas.Initialize(texConfig);
    }

    void AppendDrawItems(
        std::span<const RenderTextItem> items,
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

    FontAtlasUploadGraphBuilder& GetBuilder() noexcept { return m_atlasBuilder; }

private:
    std::vector<ShapedText> ShapeTexts(std::span<const RenderTextItem> items)
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

private:
    FontAtlasUploadGraphBuilder m_atlasBuilder;
    FontAtlas m_fontAtlas;
};