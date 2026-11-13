module DxRender.Packet;

import :UIItemBuilder;

import std;
import :UIMeshAppend;
import DxRender.Definition;
import DxRender.Text;
import DxRender.Resource;
import DxRender.TransientMesh;
import Core.Logger;
import Core.Math;
import Core.TypeHierarchy;
import Contract.Render.Backend.View;

namespace
{
    RenderTextItem ToRenderTextItem(DrawTextItem& text)
    {
        Core::Rect normalized = text.bounds;
        normalized.Normalize(); // 뒤집힌 rect 방어

        RenderTextItem item;
        item.fontRes = text.font;
        item.mode = text.mode;
        item.fontSize = text.size;
        item.position = Core::Vector2{ normalized.Left(), normalized.Top() };
        item.size = Core::Vector2{ normalized.width, normalized.height };
        item.layout = text.layout;
        item.clip = text.clip;
        item.runs = std::move(text.runs);

        return item;
    }

    // 글자 목록을 RenderTextItem으로 바꾼다. 글자가 비어 있어 건너뛴 항목은 slots에 -1로 남는다 (texts의 i번째 -> result의 위치)
    std::vector<RenderTextItem> CollectTextItems(std::vector<DrawTextItem>& texts, std::vector<std::ptrdiff_t>& slots)
    {
        std::vector<RenderTextItem> result;
        result.reserve(texts.size());
        slots.assign(texts.size(), -1);
        for (std::size_t i = 0; i < texts.size(); ++i)
        {
            if (texts[i].runs.empty())
                continue;
            if (!Core::Cast<FontResource>(texts[i].font))
            {
                Core::Logger::Error("BuildUIItems: 폰트가 없거나 폰트 리소스가 아닌 글자 항목이라 그리지 않는다 (index={})", i);
                continue;
            }
            slots[i] = static_cast<std::ptrdiff_t>(result.size());
            result.push_back(ToRenderTextItem(texts[i]));
        }
        return result;
    }

    std::optional<RenderUIItem> BakeBuffer(
        TransientMesh& transientMesh,
        UIBatchBuffer& buffer)
    {
        if (buffer.vertices.empty())
            return std::nullopt;

        auto mesh = transientMesh.Create(buffer.vertices, buffer.indices);
        if (!mesh)
            return std::nullopt;

        return RenderUIItem{ std::static_pointer_cast<IResource>(std::move(mesh)) };
    }
} // namespace

std::optional<RenderUIItem> BuildUIItems(
    OverlayViewDrawList& draws,
    TextSystem& textSystem,
    TransientMesh& transientMesh)
{
    UIBatchBuffer buffer;
    buffer.vertices.reserve(draws.ui.size() * 4);
    buffer.indices.reserve(draws.ui.size() * 6);

    std::vector<std::ptrdiff_t> textSlots;
    const auto textItems = CollectTextItems(draws.texts, textSlots);
    const auto shapedTexts = textSystem.PrepareDrawItems(textItems);

    // 호출된 순서대로 넣는다. 인덱스가 앞선 것이 먼저 그려져 아래에 깔린다
    for (const auto& entry : draws.order)
    {
        if (entry.kind == DrawOrderEntry::Kind::Image)
        {
            AppendUIItem(draws.ui[entry.index], buffer);
            continue;
        }

        const std::ptrdiff_t slot = textSlots[entry.index];
        if (slot >= 0)
            textSystem.AppendDrawItem(shapedTexts[slot], textItems[slot], buffer);
    }

    return BakeBuffer(transientMesh, buffer);
}