export module DxRender.Text:GlyphCursor;

import std;
import :TextTypes;
import :TextLayoutUtils;
import DxRender.Definition;
import DxRender.Resource;
import Core.Math;

export class GlyphCursor
{
public:
    GlyphCursor(
        const ShapedText& shaped,
        const RenderTextItem& item,
        std::span<const float> lineWidths)
        : m_item{ item }
        , m_lineWidths{ lineWidths }
    {
        float baseLineHeight = shaped.font->GetLineHeight(shaped.size);
        m_lineHeight = baseLineHeight * item.layout.lineSpacing;
        m_ascent = shaped.font->GetAscent(shaped.size);

        float totalBlockHeight = static_cast<float>(lineWidths.size()) * m_lineHeight;
        m_verticalOffset = ComputeVerticalOffset(item.layout.verticalAlign, item.size.y, totalBlockHeight);
        m_baseY = item.position.y + m_verticalOffset;

        m_baselineY = m_baseY + m_ascent;
        m_cursorX = item.position.x;
    }

    void BeginLine(std::uint32_t lineIndex) // 줄이 바뀔 때 호출 - cursorX/baselineY를 그 줄의 align에 맞게 리셋
    {
        float lineWidth = (lineIndex < m_lineWidths.size()) ? m_lineWidths[lineIndex] : 0.f;
        float alignOffset = ComputeHorizontalOffset(m_item.layout.horizontalAlign, m_item.size.x, lineWidth);

        m_cursorX = m_item.position.x + alignOffset;
        m_baselineY = m_baseY + m_ascent + static_cast<float>(lineIndex) * m_lineHeight;
    }

    // 현재 상태 조회
    float CursorX() const noexcept { return m_cursorX; }
    float BaselineY() const noexcept { return m_baselineY; }
    float VerticalOffset() const noexcept { return m_verticalOffset; }
    float BaseY() const noexcept { return m_baseY; }
    float LineHeight() const noexcept { return m_lineHeight; }

    void Advance(float advanceX) noexcept { m_cursorX += advanceX; } // glyph 하나를 그린 뒤 advance
    bool IsLineVisible(std::uint32_t lineIndex, const Core::Rect& clipRect) const // 현재 줄이 clipRect와 겹치는지 판단
    {
        float lineTop = m_baseY + static_cast<float>(lineIndex) * m_lineHeight;
        float lineBottom = lineTop + m_lineHeight;
        return !(lineBottom < clipRect.Top() || lineTop > clipRect.Bottom());
    }

private:
    const RenderTextItem& m_item;
    std::span<const float> m_lineWidths;

    float m_lineHeight{ 0.f };
    float m_ascent{ 0.f };
    float m_verticalOffset{ 0.f };
    float m_baseY{ 0.f };

    float m_cursorX{ 0.f };
    float m_baselineY{ 0.f };
};