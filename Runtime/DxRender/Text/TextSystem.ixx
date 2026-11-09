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

    // 두 단계로 나눈다. 모든 글자의 모양과 글리프를 먼저 한 번에 준비하고(아틀라스가 중간에 바뀌지 않게),
    // 그다음 그리는 순서대로 하나씩 버퍼에 넣는다. 돌려주는 목록의 i번째가 items의 i번째 글자다.
    [[nodiscard]] std::vector<ShapedText> PrepareDrawItems(const std::vector<RenderTextItem>& items);
    void AppendDrawItem(const ShapedText& shaped, const RenderTextItem& item, UIBatchBuffer& buffer);

	[[nodiscard]] std::vector<AtlasGlyphBatch> TakePendingUploads() noexcept
	{
		return m_fontAtlas.TakePendingUploads();
	}

private:
    std::vector<ShapedText> ShapeTexts(const std::vector<RenderTextItem>& items);
    FontAtlas m_fontAtlas;
};