export module UIVisualizer.SelectionOverlay;

import std;
import UserInterface;
import Contract.Render.UI;
import Contract.Render.ITextMeasurer;
import Core.Math;

// 고른 위젯과 픽한 위젯을 화면에 덧그린다 (구현: SelectionOverlay.cpp)
// - 윤곽선: 윤곽선 이미지(흰색 한 칸짜리 이미지)를 늘려 위젯 바깥쪽에 네 변을 그린다. 색은 틴트로 준다.
//   위젯 안쪽이 아니라 바깥쪽에 그려서, 작은 위젯도 윤곽선이 위젯을 가리지 않는다.
// - 선택(Draw): 빨간 윤곽선과, 위젯 왼쪽 위 안쪽에 고른 순서대로 1부터 외곽선을 준 번호.
// - 픽(DrawPick): 픽 창(체인의 위젯마다 이름이 적힌 직사각형을 위에서 아래로 쌓은 목록)을 기준 위치(클릭한 마우스 위치)에서 오른쪽 아래로 그린다.
//   창 폭은 이름 글자 폭에 맞춘다. 현재 픽의 창에는 주황 테두리를 두르고, 화면의 그 위젯에도 주황 윤곽선을 그린다.
// 브러시와 글꼴은 렌더 서비스를 초기화한 뒤 정한다. 정해지지 않은 쪽은 그리지 않는다.
export class SelectionOverlay
{
public:
    static constexpr float kThickness = 2.f;   // 윤곽선 두께(화면 픽셀). 위젯 영역 바깥쪽으로 이만큼 그린다

    static constexpr Core::Color kSelectedColor{ 1.f, 0.f, 0.f, 1.f };    // 선택한 위젯의 윤곽선
    static constexpr Core::Color kPickColor{ 1.f, 0.65f, 0.1f, 1.f };     // 지금 픽한 위젯의 윤곽선과 그 창의 테두리

    static constexpr float kPickRowMinWidth = 48.f;  // 픽 창 하나의 최소 폭. 폭은 이름 글자 폭 + 양옆 여백이고, 짧은 이름이 너무 작은 창이 되지 않게 한다
    static constexpr float kPickRowHeight = 32.f;
    static constexpr float kPickRowGap = 4.f;        // 창과 창 사이
    static constexpr float kPickTextInset = 8.f;     // 창 양옆에서 글자까지
    static constexpr std::uint32_t kPickTextSize = 16;
    static constexpr Core::Color kPickRowColor{ 0.08f, 0.10f, 0.18f, 0.85f };          // 창 배경
    static constexpr Core::Color kPickCurrentRowColor{ 0.16f, 0.19f, 0.32f, 0.95f };   // 현재 픽의 창 배경 (조금 더 밝고 진하다)

    static constexpr std::uint32_t kNumberSize = 20;
    static constexpr float kNumberInset = 2.f;       // 위젯 왼쪽 위에서 숫자 모양까지 눈에 보이는 거리 (가로, 세로 모두)
    static constexpr float kFontTopPadding = 0.25f;  // 글자 칸 맨 위에서 숫자 모양이 시작하기까지의 여백(글꼴 크기의 비율). 눈으로 맞춘 값이다. 이만큼 칸을 위로 올려야 세로 거리도 kNumberInset이 된다
    static constexpr float kNumberBoxWidth = 64.f;   // 번호를 그리는 칸의 크기. 번호 자릿수가 늘어도 넉넉하다
    static constexpr float kNumberBoxHeight = 28.f;
    static constexpr Core::Color kNumberColor{ 1.f, 1.f, 1.f, 1.f };

    void SetBrush(BrushHandle brush) { m_brush = brush; }   // 윤곽선을 그릴 이미지 (흰색 이미지. 색은 틴트로 준다)
    void SetFont(FontHandle font) { m_font = font; }
    void SetMeasurer(ITextMeasurer& measurer) { m_measurer = &measurer; }   // 픽 창의 폭을 이름 글자 폭에 맞추는 데 쓴다. measurer는 SelectionOverlay보다 오래 살아야 한다. 정하지 않으면 모든 창이 최소 폭이다

    // UI를 그린 다음 같은 오버레이에 덧그린다. items는 고른 순서대로 (번호는 순서 + 1)
    void Draw(IOverlayView& overlay, std::span<const Widget* const> items) const;

    // window의 위젯마다 창을 만들어 안쪽(앞)부터 위에서 아래로 쌓는다. 창에는 위젯 이름(없으면 종류)을 적고, 폭은 그 글자 폭에 맞춘다(왼쪽 정렬).
    // 쌓은 창 묶음은 anchor(화면 좌표)에서 오른쪽 아래로 뻗고, 화면(screen)을 넘으면 가장 넓은 창을 기준으로 안쪽으로 밀어 넣는다. current(window 안의 위젯)의 창에는 주황 테두리를 두르고 화면의 그 위젯에도 주황 윤곽선을 그린다.
    // current가 없거나 window가 비어 있으면 아무것도 그리지 않는다. 선택보다 먼저 그려서 선택이 위에 오게 한다
    void DrawPick(IOverlayView& overlay, const Core::Size& screen, std::span<const Widget* const> window, const Widget* current,
        const Core::Vector2& anchor) const;

private:
    void DrawOutline(IOverlayView& overlay, const Core::Rect& rect, const Core::Color& tint) const;
    void DrawNumber(IOverlayView& overlay, const Core::Rect& rect, std::size_t number) const;
    void DrawPickRow(IOverlayView& overlay, const Core::Rect& row, const std::string& label, bool isCurrent) const;
    float PickRowWidth(const std::string& label) const;   // 글자 폭 + 양옆 여백 (최소 kPickRowMinWidth)

    BrushHandle m_brush;
    FontHandle m_font;
    ITextMeasurer* m_measurer{ nullptr };
};
