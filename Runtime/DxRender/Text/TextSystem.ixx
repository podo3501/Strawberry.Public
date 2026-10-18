export module DxRender.Text:TextSystem;

import std;
import :TextTypes;
import :FontAtlas;
import DxRender.Definition;
import DxRender.Core;
import DxRender.Factory;
import Pipeline.GraphBuilder;
import Contract.Render.Types;

export class TextSystem
{
public:
    ~TextSystem() = default;
    TextSystem(
        Device& device,
        DescriptorFactory& factory,
        ResourceFactory& resFactory);

    bool Initialize(const TextConfig& texConfig);
    void AppendDrawItems(
        std::vector<RenderTextItem>& items,
        UIBatchBuffer& buffer);

    FontAtlasUploadGraphBuilder* GetBuilder() noexcept { return &m_atlasBuilder; }

private:
    std::vector<ShapedText> ShapeTexts(const std::vector<RenderTextItem>& items);

    FontAtlasUploadGraphBuilder m_atlasBuilder;
    FontAtlas m_fontAtlas;
};