module UIVisualizer.SelectionOverlay;

import std;

namespace
{
    // 번호 글자의 외곽선. 위젯 배경과 겹쳐도 읽히도록 어두운 외곽선을 준다
    constexpr TextOutline kNumberOutline{ TextEffectLevel::Level12, TextColor::Black };
}

void SelectionOverlay::Draw(IOverlayView& overlay, std::span<const Widget* const> items) const
{
    for (std::size_t i = 0; i < items.size(); ++i)
    {
        const Core::Rect rect = WidgetInspector::ScreenRect(*items[i]);
        DrawOutline(overlay, rect);
        DrawNumber(overlay, rect, i + 1);
    }
}

// 위젯 바깥쪽에 그린다. 위, 아래 변은 양옆 두께만큼 더 길게, 왼쪽, 오른쪽 변은 위젯 높이만큼만 그려서 모서리가 겹치지 않게 한다
void SelectionOverlay::DrawOutline(IOverlayView& overlay, const Core::Rect& rect) const
{
    if (!m_brush)
        return;

    const std::array edges{
        Core::Rect{ rect.x - kThickness, rect.y - kThickness, rect.width + kThickness * 2.f, kThickness },
        Core::Rect{ rect.x - kThickness, rect.y + rect.height, rect.width + kThickness * 2.f, kThickness },
        Core::Rect{ rect.x - kThickness, rect.y, kThickness, rect.height },
        Core::Rect{ rect.x + rect.width, rect.y, kThickness, rect.height },
    };

    for (const Core::Rect& edge : edges)
        if (edge.IsValid())   // 높이가 없는 위젯은 크기가 없는 옆 변을 건너뛴다
            overlay.DrawImage(m_brush, edge);   // 틴트 없이 이미지 색 그대로
}

void SelectionOverlay::DrawNumber(IOverlayView& overlay, const Core::Rect& rect, std::size_t number) const
{
    if (!m_font)
        return;

    TextStyle style;
    style.color = kNumberColor;
    style.outline = kNumberOutline;

    // 숫자 모양은 칸 맨 위보다 kFontTopPadding만큼 아래에서 시작하므로, 그만큼 칸을 올려서 가로와 세로의 눈에 보이는 거리를 맞춘다
    const float topPadding = kFontTopPadding * static_cast<float>(kNumberSize);
    const Core::Rect bounds{ rect.x + kNumberInset, rect.y + kNumberInset - topPadding, kNumberBoxWidth, kNumberBoxHeight };
    overlay.DrawText(m_font, TextRenderMode::MTSDF, std::to_string(number), kNumberSize, bounds, TextLayout{}, style);
}
