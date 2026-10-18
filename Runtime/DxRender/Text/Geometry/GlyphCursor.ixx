export module DxRender.Text:GlyphCursor;

import std;
import :TextTypes;
import DxRender.Definition;
import Core.Math;

export class GlyphCursor
{
public:
    ~GlyphCursor() = default;
    GlyphCursor() = delete;
    GlyphCursor(
        const ShapedText& shaped,
        const RenderTextItem& item,
        std::span<const float> lineWidths);

    void BeginLine(std::uint32_t lineIndex); // 줄이 바뀔 때 호출 - cursorX/baselineY를 그 줄의 align에 맞게 리셋
    
    // 현재 상태 조회
    float CursorX() const noexcept { return m_cursorX; }
    float BaselineY() const noexcept { return m_baselineY; }
    float VerticalOffset() const noexcept { return m_verticalOffset; }
    float BaseY() const noexcept { return m_baseY; }
    float LineHeight() const noexcept { return m_lineHeight; }

    void Advance(float advanceX) noexcept { m_cursorX += advanceX; } // glyph 하나를 그린 뒤 advance
    bool IsLineVisible(std::uint32_t lineIndex, const Core::Rect& clipRect) const; // 현재 줄이 clipRect와 겹치는지 판단

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