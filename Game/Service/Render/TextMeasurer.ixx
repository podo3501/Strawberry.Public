export module Service.Render:TextMeasurer;

import std;
import :TextRuns;
import Service.Render.Repository;
import Contract.Render.Service;
import Contract.Render.Backend;

export class TextMeasurer final : public ITextMeasurer
{
public:
    TextMeasurer(IRenderBackend& backend, RepositoryContainer& repositories)
        : m_backend{ backend }
        , m_repositories{ repositories }
    {}

    Core::Vector2 MeasureText(
        FontHandle hF, std::string_view text, std::uint32_t size,
        float maxWidth, const TextLayout& layout) override
    {
        const TextSpan spans[] = { TextSpan{ text,{} } };   // 스타일은 메트릭에 영향 없음
        return MeasureText(hF, spans, size, maxWidth, layout);
    }

    Core::Vector2 MeasureText(
        FontHandle hF, std::span<const TextSpan> spans, std::uint32_t size,
        float maxWidth, const TextLayout& layout) override
    {
        if (spans.empty())
            return {};

        auto fontRes = m_repositories.Get<FontRepository>().GetIfReady(hF);
        if (!fontRes)
            return {};   // 폰트 미로드

        return m_backend.MeasureText(fontRes, BuildTextRuns(spans), size, maxWidth, layout);
    }

private:
    RepositoryContainer& m_repositories;
    IRenderBackend& m_backend;
};