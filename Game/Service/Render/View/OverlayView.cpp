module Service.Render;

import std;
import :TextRuns;
import :OverlayView;
import Service.Render.Repository;
import Core.Assert;
import Core.Utils;

namespace
{
	// bounds의 width/height가 0이면 그 축은 무제한이다 (ComputeClipRect와 같은 규칙).
	bool BoundsOverlapClip(Core::Rect bounds, const Core::Rect& clip)
	{
		bounds.Normalize();
		const bool xOk = bounds.width <= 0.f || (bounds.Left() < clip.Right() && bounds.Right() > clip.Left());
		const bool yOk = bounds.height <= 0.f || (bounds.Top() < clip.Bottom() && bounds.Bottom() > clip.Top());
		return xOk && yOk;
	}
}

OverlayView::OverlayView(
    RepositoryContainer& repositories,
    MeshHandle uiQuad,
    BrushHandle defaultBrush)
    : RenderView{ ViewType::Overlay, repositories }
    , m_uiQuad{ uiQuad }
    , m_defaultBrush{ defaultBrush }
{}

bool OverlayView::IsEmpty() const
{
    return m_data.draws.IsEmpty();
}

void OverlayView::Reset(const OverlayViewContext& context)
{
    m_data.context = context;
    m_data.draws.Clear();
    m_clipStack.clear();
}

OverlayViewData OverlayView::TakeData()
{
    Core::Assert(m_clipStack.empty()); // PushClip/PopClip 짝이 안 맞음
    return std::move(m_data);
}

void OverlayView::DrawImage(
    BrushHandle bh,
    const Core::Rect& dest,
    const Core::Rect* source)
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

	const auto clip = CurrentClip();
	if (clip && (!clip->IsValid() || !dest.Intersects(*clip)))
		return;   // 빈 클립이거나 완전히 밖. 빈 클립을 버려야 하는 이유: 셰이더는 면적 0 클립을 "클립 없음"으로 읽는다

    float width = dest.width;
    float height = dest.height;

    Core::Matrix scale = Core::Matrix::Scale(width, height, 1.0f);
    Core::Matrix translation = Core::Matrix::Translation(dest.x, dest.y, 0.0f);
    Core::Matrix world = scale * translation;

	m_data.draws.ui.push_back(DrawUIItem{
		meshRes, brushRes, world,
		source ? std::optional<Core::Rect>(*source) : std::nullopt,
		clip });
}

void OverlayView::DrawText(
    FontHandle hF,
    TextRenderMode mode,
    std::string_view text,
    std::uint32_t size,
    const Core::Rect& bounds,
    const TextLayout& layout,
    const TextStyle& style)
{
    const TextSpan spans[] = { TextSpan{ text, style } };
    DrawText(hF, mode, spans, size, bounds, layout);
}

void OverlayView::DrawText(
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

	const auto clip = CurrentClip();
	if (clip)
	{
		if (!clip->IsValid())
			return;   // 빈 클립 아래는 아무것도 그리지 않는다

		// Clip 모드는 자기 bounds와 바깥 클립의 교집합이 빌 수 있고, 그 결과가 "클립 없음"으로 오인되므로 여기서 거른다.
		// 다른 모드는 글자가 bounds 밖으로 번질 수 있어서 bounds만으로 버리면 안 된다.
		if (layout.overflow == TextOverflow::Clip && !BoundsOverlapClip(bounds, *clip))
			return;
	}

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
		fontRes, mode, size, bounds, layout, BuildTextRuns(spans), clip
		});
}

void OverlayView::PushClip(const Core::Rect& screenRect)
{
	// 중첩하면 바깥 클립과 교집합. 교집합이 비어도 스택에는 그대로 쌓는다 (Pop 짝을 맞추기 위해)
	m_clipStack.push_back(m_clipStack.empty() ? screenRect : m_clipStack.back().Intersect(screenRect));
}

void OverlayView::PopClip()
{
	Core::Assert(!m_clipStack.empty()); // PushClip 없이 PopClip
	m_clipStack.pop_back();
}

std::optional<Core::Rect> OverlayView::CurrentClip() const
{
	if (m_clipStack.empty())
		return std::nullopt;
	return m_clipStack.back();
}