export module Contract.Render.ITextMeasurer;

import std;
import Contract.Render.Handles;
import Contract.Render.Shared.Types;
import Core.Math;

// 측정 결과는 DrawText가 그리는 레이아웃 박스와 같은 규칙을 따른다.
// maxWidth <= 0 이거나 layout.wordWrap == false 이면 줄바꿈 없음.
export struct ITextMeasurer
{
    virtual ~ITextMeasurer() = default;

    virtual Core::Vector2 MeasureText(
        FontHandle hF,
        std::string_view text,
        std::uint32_t size,
        float maxWidth = 0.f,
        const TextLayout& layout = {}) = 0;

    virtual Core::Vector2 MeasureText(
        FontHandle hF,
        std::span<const TextSpan> spans,
        std::uint32_t size,
        float maxWidth,
        const TextLayout& layout) = 0;
};