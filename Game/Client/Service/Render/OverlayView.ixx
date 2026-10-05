export module Client.Render:OverlayView;

import std;
import :RenderView;
import Client.Render.ResourceHandles;
import Client.Render.Definition;
import Client.Render.View;
import Client.Render.Repository;
import Core.Math;
import Core.Utils;
import Core.Assert;

std::vector<TextRun> BuildTextRuns(std::span<const TextSpan> spans)
{
    std::vector<TextRun> runs;
    std::uint32_t lineIndex = 0;
    for (auto& span : spans)
    {
        if (span.text.empty()) continue;
        std::vector<char32_t> codepoints = Core::UTF8ToUTF32(span.text);
        std::size_t segStart = 0;
        for (std::size_t i = 0; i <= codepoints.size(); ++i)
        {
            bool isNewline = (i < codepoints.size()) && (codepoints[i] == U'\n');
            bool isEnd = (i == codepoints.size());
            if (!isNewline && !isEnd)
                continue;
            if (i > segStart) // 빈 세그먼트(연속 \n)는 run을 만들지 않음
            {
                runs.push_back({
                    std::vector<char32_t>(codepoints.begin() + segStart, codepoints.begin() + i),
                    span.style,
                    lineIndex
                    });
            }
            if (isNewline)
                ++lineIndex; // 내용이 있든 없든 줄 번호는 증가
            segStart = i + 1;
        }
    }
    return runs;
}

namespace Render
{
    export class OverlayView : public RenderView
    {
    public:
        virtual ~OverlayView() override = default;

        OverlayView(
            RepositoryContainer& repositories,
            MeshHandle uiQuad,
            BrushHandle defaultBrush)
            : RenderView{ ViewType::Overlay, repositories }
            , m_uiQuad{ uiQuad }
            , m_defaultBrush{ defaultBrush }
        {
        }

        virtual bool IsEmpty() const override
        {
            return m_data.draws.IsEmpty();
        }

        void Reset(const OverlayViewContext& context)
        {
            m_data.context = context;
            m_data.draws.Clear();
        }

        OverlayViewData TakeData()
        {
            return std::move(m_data);
        }

        void DrawUI(
            BrushHandle bh,
            const Core::Rect& dest,
            const Core::Rect* source = nullptr)
        {
            if (!bh)
                bh = m_defaultBrush;

            auto& meshRepository = m_repositories.Get<MeshRepository>();
            auto meshRes = meshRepository.GetIfReady(m_uiQuad);
            if (!meshRes)
                return;

            auto& brushRepository = m_repositories.Get<BrushRepository>();
            auto brushRes = brushRepository.GetIfReady(bh);
            if (!brushRes)
                return;

            float width = dest.width;
            float height = dest.height;

            Core::Matrix scale = Core::Matrix::Scale(width, height, 1.0f);
            Core::Matrix translation = Core::Matrix::Translation(dest.x, dest.y, 0.0f);
            Core::Matrix world = scale * translation;

            m_data.draws.ui.push_back(DrawUIItem{
                meshRes, brushRes, world, source ? std::optional<Core::Rect>(*source) : std::nullopt
                });
        }

        void DrawText(
            FontHandle hF,
            TextRenderMode mode,
            std::string_view text,
            std::uint32_t size,
            const Core::Rect& bounds,
            const TextLayout& layout = {},
            const TextStyle& style = {})
        {
            TextSpan span{ text, style };
            DrawText(hF, mode, std::span{ &span, 1 }, size, bounds, layout);
        }

        void DrawText(
            FontHandle hF,
            TextRenderMode mode,
            std::span<const TextSpan> spans,
            std::uint32_t size,
            const Core::Rect& bounds,
            const TextLayout& layout)
        {
            if (spans.empty()) return;

            auto& fontRepository = m_repositories.Get<FontRepository>();
            auto fontRes = fontRepository.GetIfReady(hF);
            if (!fontRes)
                return;

            if (mode == TextRenderMode::Bitmap)
            {
                for (auto& span : spans)
                {
                    auto& style = span.style;
                    //비트맵에는 이 기능들이 없다. 만약 Bitmap에 기능을 추가하면 여기서 assert를 제거.
                    //아예 style을 따로 갈수도 있지만, 그러기에는 구현 비용이 크다. 그리고 bitmap이라고 이 기능이 구현이 안되는것도 아니다.
                    Core::Assert(!style.outline.has_value());
                    Core::Assert(!style.shadow.has_value());
                    Core::Assert(!style.gradient.has_value());
                    Core::Assert(!style.glow.has_value());
                }
            }

            m_data.draws.texts.push_back(DrawTextItem{
                fontRes, mode, size, bounds, layout, BuildTextRuns(spans)
                });
        }

    private:
        MeshHandle m_uiQuad;
        BrushHandle m_defaultBrush;
        OverlayViewData m_data;
    };
}