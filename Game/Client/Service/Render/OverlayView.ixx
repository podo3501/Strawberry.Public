export module Service.Render:OverlayView;

import std;
import :RenderView;
import Contract.Render.Service;
import Contract.Render.Backend.Types;
import Contract.Render.Backend.View;
import Core.Math;

export class OverlayView : public RenderView
{
public:
    virtual ~OverlayView() override = default;
    OverlayView(
        RepositoryContainer& repositories,
        MeshHandle uiQuad,
        BrushHandle defaultBrush);
    virtual bool IsEmpty() const override;

    void Reset(const OverlayViewContext& context);
    OverlayViewData TakeData();

    void DrawUI(
        BrushHandle bh,
        const Core::Rect& dest,
        const Core::Rect* source = nullptr);

    void DrawText(
        FontHandle hF,
        TextRenderMode mode,
        std::string_view text,
        std::uint32_t size,
        const Core::Rect& bounds,
        const TextLayout& layout = {},
        const TextStyle& style = {});

    void DrawText(
        FontHandle hF,
        TextRenderMode mode,
        std::span<const TextSpan> spans,
        std::uint32_t size,
        const Core::Rect& bounds,
        const TextLayout& layout);

private:
    MeshHandle m_uiQuad;
    BrushHandle m_defaultBrush;
    OverlayViewData m_data;
};