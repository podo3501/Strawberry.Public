export module Game.UserInterface.Widget:ScrollView;

import std;
import :Widget;
import :InputEvents;
import Contract.Render.IOverlayView;
import Core.Math;

// 세로 스크롤 뷰포트. 자식은 앵커로 배치된 뒤 스크롤 오프셋만큼 위로 밀리고, 뷰포트 밖은 잘린다.
// 내용 크기는 자식들이 차지하는 가장 아래 끝으로 잰다 (보통 Fit 세로 Stack 하나를 자식으로 둔다).
export class ScrollView : public Widget
{
public:
    explicit ScrollView(Core::Rect localRect) : Widget(localRect) {}

    void SetScroll(float y);                                  // 범위는 배치 때 내용 크기를 보고 맞춘다
    float Scroll() const { return m_scrollY; }
    void SetLineStep(float step) { m_lineStep = step; }       // 휠 한 칸당 이동량(px)

protected:
    bool IsHitTestable() const override { return true; }      // 자식이 없는 빈 곳에서도 휠을 받는다
    bool OnScroll(const ScrollEvent& e) override;

    bool ClipsChildren() const override { return true; }
    void DrawSelf(IOverlayView&, const Core::Rect&) const override {}
    void ArrangeChildren() override;

private:
    float m_scrollY{ 0.f };
    float m_lineStep{ 40.f };
};