export module Runtime.Render.Text:UnderlineBatcher;

import std;
import :TextTypes;
import :FontAtlas;
import :TextGeometryUtils;
import Runtime.Render.Definition;
import Runtime.Render.Resource;
import Core.Math;
import Client.Render.Definition;

export class UnderlineBatcher
{
public:
    UnderlineBatcher(
        const FontAtlas& atlas,
        const ShapedText& shaped,
        UIBatchBuffer& buffer,
        const Core::Rect& clipRect)
        : m_atlas{ atlas }
        , m_shaped{ shaped }
        , m_buffer{ buffer }
        , m_clipRect{ clipRect }
    {
    }

    // 현재 glyph의 style을 보고, 필요시 구간을 열거나 닫는다.
    void Update(const TextStyle& style, float cursorX, float baselineY)
    {
        bool wantsUnderline = style.underline.has_value();
        Core::Color wantsColor = wantsUnderline
            ? style.underline->color.value_or(style.color)
            : Core::Color{};
        float wantsThickness = wantsUnderline
            ? (style.underline->thickness > 0.f
                ? style.underline->thickness
                : m_shaped.font->GetUnderlineThickness(m_shaped.size))
            : 0.f;

        bool changed = m_active &&
            (!wantsUnderline || wantsColor != m_color || wantsThickness != m_thickness);

        if (changed)
            Flush(cursorX, baselineY);

        if (!m_active && wantsUnderline)
        {
            m_active = true;
            m_startX = cursorX;
            m_color = wantsColor;
            m_thickness = wantsThickness;
        }
    }

    // 진행 중인 구간을 quad로 확정하고 닫는다. (줄 전환, 순회 종료 시에도 호출)
    void Flush(float endX, float baselineY)
    {
        if (!m_active)
            return;
        m_active = false;

        float width = endX - m_startX;
        if (width <= 0.f || m_thickness <= 0.f)
            return;

        float underlineY = baselineY + m_shaped.font->GetUnderlineOffset(m_shaped.size);
        Core::Rect underlineRect{ m_startX, underlineY, width, m_thickness };
        if (m_clipRect.IsValid())
        {
            if (!underlineRect.Intersects(m_clipRect)) // clipRect와 교집합해서 실제로 보일 영역만큼만 quad를 만든다. 완전히 밖이면 아예 push하지 않음.
                return;
            underlineRect = underlineRect.Intersect(m_clipRect);
        }

        const auto& brush = m_atlas.GetSolidBrush();
        const auto textureIndex = brush->GetTextureIndex();
        AppendSolidQuad(
            m_buffer,
            textureIndex,
            underlineRect,
            m_color,
            m_clipRect); // clipRect가 유효할 때는 이미 이 안에 완전히 들어가는 quad이므로 셰이더 discard는 안전망 역할만.
    }

private:
    const FontAtlas& m_atlas;
    const ShapedText& m_shaped;
    UIBatchBuffer& m_buffer;
    Core::Rect m_clipRect;

    bool m_active{ false };
    float m_startX{ 0.f };

    Core::Color m_color{};
    float m_thickness{ 0.f };
};