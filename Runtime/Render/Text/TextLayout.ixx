module;

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

export module Runtime.Render.Text:TextLayout;

import std;
import :TextTypes;
import Runtime.Render.Resource;
import Core.Assert;

namespace
{
    bool IsCJK(char32_t cp)
    {
        return (cp >= 0x1100 && cp <= 0x11FF)   // 한글 자모 (조합형)
            || (cp >= 0x3040 && cp <= 0x30FF)   // 히라가나 + 가타카나
            || (cp >= 0x3130 && cp <= 0x318F)   // 한글 호환 자모
            || (cp >= 0x3400 && cp <= 0x4DBF)   // CJK 확장 한자 A
            || (cp >= 0x4E00 && cp <= 0x9FFF)   // CJK 통합 한자
            || (cp >= 0xAC00 && cp <= 0xD7A3)   // 한글 음절 (가~힣)
            || (cp >= 0xF900 && cp <= 0xFAFF);  // CJK 호환용 한자
    }

    bool IsWhitespace(char32_t cp)
    {
        return cp == U' ' || cp == U'\t';
    }

    struct WrapState
    {
        std::uint32_t currentLine;
        float lineWidth;
        std::optional<std::size_t> lastBreak;
    };

    // [breakAt, i] 구간을 새 줄로 시작한다. breakAt이 공백이면 그 공백은 어느 줄의 너비 계산에도 넣지 않고 건너뛴다.
    void StartNewLine(
        std::span<ShapedGlyph> glyphs,
        std::size_t breakAt, std::size_t i,
        WrapState& state)
    {
        if (IsWhitespace(glyphs[breakAt].codepoint))
        {
            glyphs[breakAt].lineIndex = state.currentLine;
            ++breakAt;
        }

        ++state.currentLine;
        state.lineWidth = 0.f;
        for (std::size_t k = breakAt; k <= i; ++k)
        {
            glyphs[k].lineIndex = state.currentLine;
            state.lineWidth += glyphs[k].advanceX;
        }

        state.lastBreak.reset();
    }

    std::uint32_t ApplyWordWrapToRange(
        std::span<ShapedGlyph> glyphs,
        std::size_t start, std::size_t end,
        float maxWidth,
        std::uint32_t baseLineIndex)
    {
        if (maxWidth <= 0.f)
        {
            for (std::size_t i = start; i < end; ++i)
                glyphs[i].lineIndex = baseLineIndex;
            return baseLineIndex;
        }

        WrapState state{ baseLineIndex, 0.f, std::nullopt };

        for (std::size_t i = start; i < end; ++i)
        {
            if (IsWhitespace(glyphs[i].codepoint) || IsCJK(glyphs[i].codepoint))
                state.lastBreak = i;

            float glyphWidth = glyphs[i].advanceX;

            // 브레이크 지점이 없고, 이 glyph 하나만으로도 줄이 넘치는 경우 -> 문자 단위 강제 컷
            if (state.lineWidth + glyphWidth > maxWidth && i > start && !state.lastBreak.has_value())
            {
                StartNewLine(glyphs, i, i, state);
                continue;
            }

            state.lineWidth += glyphWidth;

            if (state.lineWidth > maxWidth && i > start)
            {
                std::size_t breakAt = state.lastBreak.value_or(i);
                StartNewLine(glyphs, breakAt, i, state);
                continue;
            }

            glyphs[i].lineIndex = state.currentLine;
        }

        return state.currentLine;
    }

    std::vector<ShapedGlyph> ShapeText(
        FontResource* font, std::span<const char32_t> text, std::uint32_t size)
    {
        hb_font_t* hbFont = font->GetHbFont(size);
        if (!hbFont) return {};

        hb_buffer_t* buffer = hb_buffer_create();
        hb_buffer_add_utf32(
            buffer,
            reinterpret_cast<const std::uint32_t*>(text.data()),
            static_cast<int>(text.size()), 0, static_cast<int>(text.size()));

        hb_buffer_guess_segment_properties(buffer);
        hb_shape(hbFont, buffer, nullptr, 0);

        unsigned glyphCount = 0;
        const hb_glyph_info_t* infos = hb_buffer_get_glyph_infos(buffer, &glyphCount);
        const hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buffer, &glyphCount);

        std::vector<ShapedGlyph> result;
        result.reserve(glyphCount);
        for (unsigned i = 0; i < glyphCount; ++i)
        {
            ShapedGlyph glyph;
            glyph.glyphIndex = infos[i].codepoint;
            glyph.sourceIndex = infos[i].cluster;
            glyph.advanceX = positions[i].x_advance / 64.f;
            glyph.offsetX = positions[i].x_offset / 64.f;
            glyph.offsetY = positions[i].y_offset / 64.f;
            result.push_back(glyph);
        }

        hb_buffer_destroy(buffer);
        return result;
    }
} //namespace

export void ApplyWordWrap(std::span<ShapedGlyph> glyphs, float maxWidth, bool wordWrap)
{
    if (!wordWrap || glyphs.empty())
        return;

    std::size_t start = 0;
    std::uint32_t lineOffset = 0; // wrap으로 늘어난 만큼 뒤 hard-line들을 밀어줌

    while (start < glyphs.size())
    {
        std::uint32_t hardLine = glyphs[start].lineIndex;
        std::size_t end = start;
        while (end < glyphs.size() && glyphs[end].lineIndex == hardLine)
            ++end;

        std::uint32_t lastLine = ApplyWordWrapToRange(
            glyphs, start, end, maxWidth, hardLine + lineOffset);

        lineOffset = lastLine - hardLine;
        start = end;
    }
}

export std::vector<ShapedGlyph> ShapeRuns(
    FontResource* font,
    std::span<const TextRun> runs,
    std::uint32_t fontSize)
{
    std::vector<ShapedGlyph> result;

    std::size_t runIdx = 0;

    while (runIdx < runs.size())
    {
        std::uint32_t lineIndex = runs[runIdx].lineIndex;
        std::size_t groupStart = runIdx;

        std::vector<char32_t> combined;
        std::vector<std::size_t> runStarts;

        while (runIdx < runs.size() &&
            runs[runIdx].lineIndex == lineIndex)
        {
            runStarts.push_back(combined.size());

            combined.insert(
                combined.end(),
                runs[runIdx].codePoints.begin(),
                runs[runIdx].codePoints.end());

            ++runIdx;
        }

        auto glyphs = ShapeText(font, combined, fontSize);

        for (auto& glyph : glyphs)
        {
            auto it = std::ranges::upper_bound(runStarts, glyph.sourceIndex);
            std::uint32_t localRun = static_cast<std::uint32_t>(std::distance(runStarts.begin(), it) - 1);

            glyph.runIndex = static_cast<std::uint32_t>(groupStart + localRun);
            glyph.lineIndex = lineIndex;
            Core::Assert(glyph.sourceIndex < combined.size()); // font->Shape()가 유효 범위를 벗어난 sourceIndex를 반환
            glyph.codepoint = (glyph.sourceIndex < combined.size())
                ? combined[glyph.sourceIndex]
                : U'\0';

            result.push_back(std::move(glyph));
        }
    }

    return result;
}