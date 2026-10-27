export module Game.UserInterface.Widget:Text;

import :Widget;
import Contract.Render.UI;
import Core.Math;

export class Text final : public Widget
{
public:
    Text(FontHandle font, TextRenderMode mode, std::string text,
        std::uint32_t size, Core::Rect localBounds,
        TextLayout layout = {}, TextStyle style = {})
        : Widget(localBounds), m_font(font), m_mode(mode)
        , m_text(std::move(text)), m_size(size)
        , m_layout(layout), m_style(style) {}

protected:
    void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const override
    {
        overlay.DrawText(m_font, m_mode, m_text, m_size, screenRect, m_layout, m_style);
    }

private:
    FontHandle m_font;
    TextRenderMode m_mode;
    std::string m_text;
    std::uint32_t m_size;
    TextLayout m_layout;
    TextStyle m_style;
};