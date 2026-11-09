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
    std::optional<Core::Rect> clip;
    Core::Color tint;
};

export struct DrawTextItem
{
    std::shared_ptr<IResource> font;
    TextRenderMode mode;
    std::uint32_t size;
    Core::Rect bounds;
    TextLayout layout;
    std::vector<TextRun> runs;
    std::optional<Core::Rect> clip;
};

// 이미지와 글자는 목록이 따로지만, order가 호출된 순서를 기억해 나중에 그린 것이 위에 오게 한다.
export struct DrawOrderEntry
{
    enum class Kind : std::uint8_t { Image, Text };

    Kind kind;
    std::uint32_t index;    // kind에 맞는 목록(ui 또는 texts) 안의 위치
};

export struct OverlayViewDrawList
{
    std::vector<DrawUIItem> ui;
    std::vector<DrawTextItem> texts;
    std::vector<DrawOrderEntry> order;

    void Add(DrawUIItem item)
    {
        order.push_back({ DrawOrderEntry::Kind::Image, static_cast<std::uint32_t>(ui.size()) });
        ui.push_back(std::move(item));
    }

    void Add(DrawTextItem item)
    {
        order.push_back({ DrawOrderEntry::Kind::Text, static_cast<std::uint32_t>(texts.size()) });
        texts.push_back(std::move(item));
    }

    bool IsEmpty() const
    {
        return order.empty();
    }

    void Clear()
    {
        ui.clear();
        texts.clear();
        order.clear();
    }
};