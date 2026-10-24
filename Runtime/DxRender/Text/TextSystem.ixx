export module DxRender.Text:TextSystem;

import std;
import :TextTypes;
import :FontAtlas;
import DxRender.Definition;
import DxRender.Core;
import DxRender.Factory;
import Contract.Render.Backend.Types;

export class TextSystem
{
public:
    ~TextSystem() = default;
    TextSystem(Device& device, DescriptorFactory& factory);

    bool Initialize(const TextConfig& texConfig);
    void AppendDrawItems(
        std::vector<RenderTextItem>& items,
        UIBatchBuffer& buffer);

	[[nodiscard]] std::vector<AtlasGlyphBatch> TakePendingUploads() noexcept
	{
		return m_fontAtlas.TakePendingUploads();
	}

private:
    std::vector<ShapedText> ShapeTexts(const std::vector<RenderTextItem>& items);
    FontAtlas m_fontAtlas;
};