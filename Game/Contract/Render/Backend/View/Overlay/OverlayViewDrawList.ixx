export module Contract.Render.Backend.View:OverlayDrawList;

import std;
import Core.Math;
import Contract.Render.Shared.Types;
import Contract.Render.IResource;

export struct DrawUIItem
{
    std::shared_ptr<IResource> mesh;
    std::shared_ptr<IResource> brush;
    Core::Matrix world;
    std::optional<Core::Rect> source;
};

export struct DrawTextItem
{
    std::shared_ptr<IResource> font;
    TextRenderMode mode;
    std::uint32_t size;
    Core::Rect bounds;
    TextLayout layout;
    std::vector<TextRun> runs;
};

export struct OverlayViewDrawList
{
    std::vector<DrawUIItem> ui;
    std::vector<DrawTextItem> texts;

    bool IsEmpty() const
    {
        return ui.empty() && texts.empty();
    }

    void Clear()
    {
        ui.clear();
        texts.clear();
    }
};