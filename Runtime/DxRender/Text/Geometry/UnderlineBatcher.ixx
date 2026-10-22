export module DxRender.Text:UnderlineBatcher;

import std;
import :TextTypes;
import :FontAtlas;
import DxRender.Definition;
import Core.Math;
import Contract.Render.Backend.Types;

export class UnderlineBatcher
{
public:
    ~UnderlineBatcher() = default;
    UnderlineBatcher() = delete;
    UnderlineBatcher(
        const FontAtlas& atlas,
        const ShapedText& shaped,
        UIBatchBuffer& buffer,
        const Core::Rect& clipRect);
    
    void Update(const TextStyle& style, float cursorX, float baselineY); // 현재 glyph의 style을 보고, 필요시 구간을 열거나 닫는다.
    void Flush(float endX, float baselineY); // 진행 중인 구간을 quad로 확정하고 닫는다. (줄 전환, 순회 종료 시에도 호출)

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