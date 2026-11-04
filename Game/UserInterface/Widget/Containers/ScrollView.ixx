export module Game.UserInterface.Widget:ScrollView;

import std;
import :Widget;
import :InputEvents;
import Contract.Render.UI;
import Contract.Render.IOverlayView;
import Core.Math;

// 세로 스크롤 뷰포트. 자식은 앵커로 배치된 뒤 스크롤 오프셋만큼 위로 밀리고, 뷰포트 밖은 잘린다.
// 내용 크기는 자식들이 차지하는 가장 아래 끝으로 잰다 (보통 Fit 세로 Stack 하나를 자식으로 둔다).
//
// SetScrollBar를 부르면 오른쪽 가장자리에 세로 스크롤 막대(트랙 + 손잡이)가 자식 위에 그려진다.
// 내용이 뷰포트보다 클 때만 보이고, 손잡이 길이는 (뷰포트 / 내용) 비율이다.
// 손잡이를 잡고 끌거나, 트랙을 누르면 손잡이가 그 자리로 점프한다 (누른 채 끌면 계속 따라간다).
// 막대는 자식이 입력을 소비하지 않는 곳에서만 눌린다. 내용이 막대 자리를 덮지 않도록 막대 폭만큼 비워 둔다
// (예: 내용 폭 = 뷰포트 폭 - 막대 폭).
export struct ScrollBarSkin
{
    BrushHandle track{};
    BrushHandle thumb{};
    float width{ 8.f };
    float minThumbLength{ 16.f };   // 내용이 아주 길어도 손잡이를 잡을 수 있게 하는 최소 길이
};

export class ScrollView : public Widget
{
public:
    explicit ScrollView(Core::Rect localRect) : Widget(localRect) {}

    void SetScroll(float y);                                  // 범위는 배치 때 내용 크기를 보고 맞춘다
    float Scroll() const { return m_scrollY; }
    void SetLineStep(float step) { m_lineStep = step; }       // 휠 한 칸당 이동량(px)
    void SetScrollBar(ScrollBarSkin skin) { m_bar = skin; }

protected:
    bool IsHitTestable() const override { return true; }      // 자식이 없는 빈 곳에서도 휠을 받는다
    bool OnScroll(const ScrollEvent& e) override;
    bool OnPointerDown(const PointerEvent& e) override;       // 막대 위일 때만 소비(캡처)한다
    void OnPointerMove(const PointerEvent& e) override;
    void OnPointerUp(const PointerEvent&) override { m_dragging = false; }

    bool ClipsChildren() const override { return true; }
    void DrawSelf(IOverlayView&, const Core::Rect&) const override {}
    void DrawAfterChildren(IOverlayView& overlay, const Core::Rect& screenRect) const override;
    void ArrangeChildren() override;

private:
    struct BarLayout   // 화면 좌표의 막대 모양
    {
        Core::Rect track;
        Core::Rect thumb;
        float travel;   // 손잡이가 움직이는 거리
    };

    std::optional<BarLayout> Bar() const;   // 막대가 없거나 내용이 뷰포트에 다 들어가면 nullopt
    void DragTo(float pointerY);

    float m_scrollY{ 0.f };
    float m_lineStep{ 40.f };
    float m_contentHeight{ 0.f };   // 마지막 배치 때 잰 값
    std::optional<ScrollBarSkin> m_bar;
    bool m_dragging{ false };
    float m_grab{ 0.f };            // 손잡이 위쪽 끝에서 잡은 지점까지의 거리
};