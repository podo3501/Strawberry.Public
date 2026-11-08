export module UIVisualizer.StatusOverlay;

import std;
import Contract.Render.UI;
import Core.Math;

// 게임 화면 위에 UIVisualizer의 상태를 덧그린다 (구현: StatusOverlay.cpp)
// - 레이아웃을 읽지 못한 오류가 있으면 화면 왼쪽 위에 오류 색 글자로 안내한다 (UIVisualizer 모드와 상관없이 보인다)
// - UIVisualizer 모드이면 화면 오른쪽 아래에 모드 이름을 노란 글자로 적는다
// ImGui가 아니라 렌더 서비스의 글자로 그리므로 UI와 같은 화면 안에서 보인다. 글꼴은 렌더 서비스를 초기화한 뒤 SetFont로 정한다.
export class StatusOverlay
{
public:
    static constexpr std::uint32_t kFontSize = 16;
    static constexpr float kMargin = 8.f;   // 화면 가장자리에서 띄우는 거리
    static constexpr std::string_view kModeLabel = "UIVisualizer 모드";
    static constexpr Core::Color kErrorColor{ 1.f, 0.35f, 0.35f, 1.f };
    static constexpr Core::Color kModeColor{ 1.f, 0.85f, 0.1f, 1.f };

    // 글꼴이 정해지기 전에는 아무것도 그리지 않는다
    void SetFont(FontHandle font) { m_font = font; }

    // UI를 그린 다음 같은 오버레이에 덧그린다. error가 비어 있으면 오류 안내는 그리지 않는다
    void Draw(IOverlayView& overlay, const Core::Size& screen, bool visualMode, std::string_view error) const;

private:
    FontHandle m_font;
};
