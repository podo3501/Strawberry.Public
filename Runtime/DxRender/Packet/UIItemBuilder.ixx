export module DxRender.Packet:UIItemBuilder;

import std;
import :UIMeshAppend;
import DxRender.Definition;
import DxRender.Text;
import DxRender.Provider;
import DxRender.Resource;
import Core.Assert;
import Core.Math;
import Contract.Render.View;

namespace
{
    RenderTextItem ToRenderTextItem(DrawTextItem& text)
    {
        Core::Assert(text.font);
        Core::Rect normalized = text.bounds;
        normalized.Normalize(); // 뒤집힌 rect 방어

        RenderTextItem item;
        item.fontRes = text.font;
        item.mode = text.mode;
        item.fontSize = text.size;
        item.position = Core::Vector2{ normalized.Left(), normalized.Top() };
        item.size = Core::Vector2{ normalized.width, normalized.height };
        item.layout = text.layout;
        item.runs = std::move(text.runs);

        return item;
    }

    std::vector<RenderTextItem> CollectTextItems(std::vector<DrawTextItem>& texts)
    {
        std::vector<RenderTextItem> result;
        result.reserve(texts.size());
        for (auto& text : texts)
        {
            if (text.runs.empty())
                continue;
            result.push_back(ToRenderTextItem(text));
        }
        return result;
    }

    std::optional<RenderUIItem> BakeBuffer(
        TransientMeshProvider& meshProvider,
        UIBatchBuffer& buffer)
    {
        if (buffer.vertices.empty())
            return std::nullopt;

        auto mesh = meshProvider.Create(buffer.vertices, buffer.indices);
        if (!mesh)
            return std::nullopt;

        return RenderUIItem{ std::static_pointer_cast<IResource>(std::move(mesh)) };
    }
} // 익명 네임스페이스

export std::optional<RenderUIItem> BuildUIItems(
    OverlayViewDrawList& draws,
    TextSystem& textSystem,
    TransientMeshProvider& meshProvider)
{
    UIBatchBuffer buffer;

    if (!draws.ui.empty())
        AppendUIItems(draws.ui, buffer);

    if (!draws.texts.empty())
    {
        auto textItems = CollectTextItems(draws.texts);
        textSystem.AppendDrawItems(textItems, buffer);
    }

    return BakeBuffer(meshProvider, buffer);
}