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
        DrawOutline(overlay, rect, kSelectedColor);
        DrawNumber(overlay, rect, i + 1);
    }
}

void SelectionOverlay::DrawPick(IOverlayView& overlay, const Core::Size& screen, std::span<const Widget* const> window, const Widget* current,
    const Core::Vector2& anchor) const
{
    if (!current || window.empty())
        return;

    DrawOutline(overlay, WidgetInspector::ScreenRect(*current), kPickColor);   // 화면의 어느 위젯인지 바로 보이게 한다

    // 창마다 이름과 폭을 정한다
    std::vector<std::string> labels;
    float widest = 0.f;
    for (const Widget* widget : window)
    {
        const std::string& name = WidgetInspector::Name(*widget);
        labels.push_back(name.empty() ? WidgetInspector::TypeName(*widget) : name);
        widest = std::max(widest, PickRowWidth(labels.back()));
    }

    // 기준 위치에서 오른쪽 아래로 시작하되 화면을 넘지 않게 민다
    const float count = static_cast<float>(window.size());
    const float totalHeight = count * kPickRowHeight + (count - 1.f) * kPickRowGap;
    const float x = std::clamp(anchor.x, 0.f, std::max(0.f, static_cast<float>(screen.width) - widest));
    const float y = std::clamp(anchor.y, 0.f, std::max(0.f, static_cast<float>(screen.height) - totalHeight));

    for (std::size_t i = 0; i < window.size(); ++i)
    {
        const Core::Rect row{ x, y + static_cast<float>(i) * (kPickRowHeight + kPickRowGap), PickRowWidth(labels[i]), kPickRowHeight };
        DrawPickRow(overlay, row, labels[i], window[i] == current);
    }
}

float SelectionOverlay::PickRowWidth(const std::string& label) const
{
    if (!m_measurer || !m_font)
        return kPickRowMinWidth;

    TextLayout layout;
    layout.wordWrap = false;
    const float textWidth = m_measurer->MeasureText(m_font, label, kPickTextSize, 0.f, layout).x;
    return std::max(kPickRowMinWidth, std::ceil(textWidth) + kPickTextInset * 2.f);
}

void SelectionOverlay::DrawPickRow(IOverlayView& overlay, const Core::Rect& row, const std::string& label, bool isCurrent) const
{
    if (m_brush)
        overlay.DrawImage(m_brush, row, nullptr, isCurrent ? kPickCurrentRowColor : kPickRowColor);
    if (isCurrent)
        DrawOutline(overlay, row, kPickColor);

    if (!m_font)
        return;

    TextLayout layout;
    layout.verticalAlign = TextVerticalAlign::Middle;
    layout.wordWrap = false;
    TextStyle style;
    style.color = kNumberColor;
    const Core::Rect bounds{ row.x + kPickTextInset, row.y, row.width - kPickTextInset * 2.f, row.height };
    overlay.DrawText(m_font, TextRenderMode::MTSDF, label, kPickTextSize, bounds, layout, style);
}

// 위젯 바깥쪽에 그린다. 위, 아래 변은 양옆 두께만큼 더 길게, 왼쪽, 오른쪽 변은 위젯 높이만큼만 그려서 모서리가 겹치지 않게 한다
void SelectionOverlay::DrawOutline(IOverlayView& overlay, const Core::Rect& rect, const Core::Color& tint) const
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
            overlay.DrawImage(m_brush, edge, nullptr, tint);   // 흰색 이미지에 틴트로 색을 준다
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
