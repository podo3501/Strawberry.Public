export module UIVisualizer.SelectionOverlay;

import std;
import UserInterface;
import Contract.Render.UI;
import Contract.Render.ITextMeasurer;
import Core.Math;

// 고른 위젯과 픽한 위젯을 화면에 덧그린다 (구현: SelectionOverlay.cpp)
// - 선택(Draw)과 픽 중인 위젯(DrawPick)은 흰색 한 칸 이미지를 위젯 크기로 늘려 밝은 회색 반투명 면으로 덮는다. 선택에는 왼쪽 위에 번호를 더한다.
// - 픽 창: 체인의 위젯 이름이 적힌 직사각형을 기준 위치에서 오른쪽 아래로 쌓는다. 폭은 이름 글자 폭에 맞추고 현재 픽의 창에는 1px 주황 테두리를 두른다.
// 브러시와 글꼴은 렌더 서비스를 초기화한 뒤 정하고, 정해지지 않은 쪽은 그리지 않는다.
export class SelectionOverlay
{
public:
    static constexpr Core::Color kHighlightColor{ 1.f, 1.f, 1.f, 0.08f };   // 선택과 픽 중인 위젯을 덮는 밝은 회색 반투명 면
    static constexpr Core::Color kPickColor{ 1.f, 0.65f, 0.1f, 1.f };         // 현재 픽의 창 테두리
    static constexpr float kPickRowBorderThickness = 1.f;                      // 현재 픽 창의 테두리 두께

    static constexpr float kPickRowMinWidth = 48.f;  // 픽 창 하나의 최소 폭. 폭은 이름 글자 폭 + 양옆 여백이고, 짧은 이름이 너무 작은 창이 되지 않게 한다
    static constexpr float kPickRowHeight = 32.f;
    static constexpr float kPickRowGap = 4.f;        // 창과 창 사이
    static constexpr float kPickTextInset = 8.f;     // 창 양옆에서 글자까지
    static constexpr std::uint32_t kPickTextSize = 16;
    static constexpr Core::Color kPickRowColor{ 0.16f, 0.19f, 0.30f, 0.95f };          // 체인 창 배경. 현재 픽은 배경이 아니라 주황 테두리로 구분한다

    static constexpr std::uint32_t kNumberSize = 20;
    static constexpr float kNumberInset = 2.f;       // 위젯 왼쪽 위에서 숫자 모양까지 눈에 보이는 거리 (가로, 세로 모두)
    static constexpr float kFontTopPadding = 0.25f;  // 글자 칸 맨 위에서 숫자 모양이 시작하기까지의 여백(글꼴 크기의 비율). 눈으로 맞춘 값이다. 이만큼 칸을 위로 올려야 세로 거리도 kNumberInset이 된다
    static constexpr float kNumberBoxWidth = 64.f;   // 번호를 그리는 칸의 크기. 번호 자릿수가 늘어도 넉넉하다
    static constexpr float kNumberBoxHeight = 28.f;
    static constexpr Core::Color kNumberColor{ 1.f, 1.f, 1.f, 1.f };

    void SetBrush(BrushHandle brush) { m_brush = brush; }   // 면과 창을 그릴 이미지 (흰색 이미지. 색은 틴트로 준다)
    void SetFont(FontHandle font) { m_font = font; }
    void SetMeasurer(ITextMeasurer& measurer) { m_measurer = &measurer; }   // 픽 창의 폭을 글자 폭에 맞추는 데 쓴다. 정하지 않으면 모든 창이 최소 폭이다

    // 고른 순서대로 (번호는 순서 + 1) 반투명 면과 번호를 UI 위에 덧그린다
    void Draw(IOverlayView& overlay, std::span<const Widget* const> items) const;

    // current(window 안의 위젯)를 반투명 면으로 덮고, window의 위젯 이름 창을 anchor(화면 좌표)에서 오른쪽 아래로 쌓는다 (화면을 넘으면 안쪽으로 민다).
    // current가 없거나 window가 비어 있으면 아무것도 그리지 않는다. 선택보다 먼저 부르면 선택이 위에 온다
    void DrawPick(IOverlayView& overlay, const Core::Size& screen, std::span<const Widget* const> window, const Widget* current,
        const Core::Vector2& anchor) const;

private:
    void DrawFill(IOverlayView& overlay, const Core::Rect& rect) const;
    void DrawBorder(IOverlayView& overlay, const Core::Rect& rect, const Core::Color& tint, float thickness) const;   // rect 바깥쪽에 두른다
    void DrawNumber(IOverlayView& overlay, const Core::Rect& rect, std::size_t number) const;
    void DrawPickRow(IOverlayView& overlay, const Core::Rect& row, const std::string& label, bool isCurrent) const;
    float PickRowWidth(const std::string& label) const;   // 글자 폭 + 양옆 여백 (최소 kPickRowMinWidth)

    BrushHandle m_brush;
    FontHandle m_font;
    ITextMeasurer* m_measurer{ nullptr };
};
