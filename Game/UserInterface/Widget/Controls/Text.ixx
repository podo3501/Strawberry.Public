export module Game.UserInterface.Widget:Text;

import :Widget;
import Contract.Render.UI;
import Contract.Render.ITextMeasurer;
import Core.Math;

export class Text final : public Widget
{
public:
    Text(ITextMeasurer& measurer, FontHandle font, TextRenderMode mode, std::string text,
        std::uint32_t size, Core::Rect localBounds,
        TextLayout layout = {}, TextStyle style = {})
        : Widget(localBounds), m_measurer(measurer), m_font(font), m_mode(mode)
        , m_text(std::move(text)), m_size(size)
        , m_layout(layout), m_style(style)
        , m_wrapWidth(layout.wordWrap ? localBounds.width : 0.f) 
    {}

    void SetText(std::string text)
    {
	    if (text == m_text)
		    return;

	    m_text = std::move(text);
        m_measured.reset();
	    InvalidateLayout();   // Fit이면 크기를 다시 재고, 부모/형제를 재배치
    }

protected:
    void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const override
    {
        overlay.DrawText(m_font, m_mode, m_text, m_size, screenRect, m_layout, m_style);
    }

	Core::Vector2 MeasureContent() const override
	{
        if (!m_measured)
        {
            const Core::Vector2 size = m_measurer.MeasureText(m_font, m_text, m_size, m_wrapWidth, m_layout);
            if (size.x <= 0.f && !m_text.empty())
                return size;   // 폰트 미로드 등으로 실패했을 수 있는 측정은 캐시하지 않는다
            m_measured = size;
        }
        return *m_measured;
	}

private:
    ITextMeasurer& m_measurer;
    FontHandle m_font;
    TextRenderMode m_mode;
    std::string m_text;
    std::uint32_t m_size;
    TextLayout m_layout;
    TextStyle m_style;
	float m_wrapWidth;   // 줄바꿈 폭: 생성 시 bounds.width (Fit이 width를 덮어써도 바뀌지 않음)
    mutable std::optional<Core::Vector2> m_measured;   // 폰트/크기/레이아웃/줄바꿈 폭이 바뀌는 setter를 추가하면 거기서도 reset할 것
};