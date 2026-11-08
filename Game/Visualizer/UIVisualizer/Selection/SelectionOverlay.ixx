export module UIVisualizer.SelectionOverlay;

import std;
import UserInterface;
import Contract.Render.UI;
import Core.Math;

// 고른 위젯에 윤곽선과 번호를 덧그린다 (구현: SelectionOverlay.cpp)
// - 윤곽선: 윤곽선 이미지(브러시, 색이 입혀진 이미지)를 늘려 위젯 바깥쪽에 네 변을 그린다. 이미지 색 그대로 그리므로 색은 이미지가 정한다.
//   위젯 안쪽이 아니라 바깥쪽에 그려서, 작은 위젯도 윤곽선이 위젯을 가리지 않는다.
// - 번호: 위젯 왼쪽 위 안쪽에 고른 순서대로 1부터, 외곽선을 준 글자로 적는다.
// 브러시와 글꼴은 렌더 서비스를 초기화한 뒤 정한다. 정해지지 않은 쪽은 그리지 않는다.
export class SelectionOverlay
{
public:
    static constexpr float kThickness = 2.f;   // 윤곽선 두께(화면 픽셀). 위젯 영역 바깥쪽으로 이만큼 그린다

    static constexpr std::uint32_t kNumberSize = 20;
    static constexpr float kNumberInset = 2.f;       // 위젯 왼쪽 위에서 숫자 모양까지 눈에 보이는 거리 (가로, 세로 모두)
    static constexpr float kFontTopPadding = 0.25f;  // 글자 칸 맨 위에서 숫자 모양이 시작하기까지의 여백(글꼴 크기의 비율). 눈으로 맞춘 값이다. 이만큼 칸을 위로 올려야 세로 거리도 kNumberInset이 된다
    static constexpr float kNumberBoxWidth = 64.f;   // 번호를 그리는 칸의 크기. 번호 자릿수가 늘어도 넉넉하다
    static constexpr float kNumberBoxHeight = 28.f;
    static constexpr Core::Color kNumberColor{ 1.f, 1.f, 1.f, 1.f };

    void SetBrush(BrushHandle brush) { m_brush = brush; }   // 윤곽선을 그릴 이미지 (빨간 이미지 등)
    void SetFont(FontHandle font) { m_font = font; }

    // UI를 그린 다음 같은 오버레이에 덧그린다. items는 고른 순서대로 (번호는 순서 + 1)
    void Draw(IOverlayView& overlay, std::span<const Widget* const> items) const;

private:
    void DrawOutline(IOverlayView& overlay, const Core::Rect& rect) const;
    void DrawNumber(IOverlayView& overlay, const Core::Rect& rect, std::size_t number) const;

    BrushHandle m_brush;
    FontHandle m_font;
};
