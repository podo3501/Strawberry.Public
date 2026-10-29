export module Game.UserInterface.Widget:Stack;

import std;
import :Widget;
import Contract.Render.IOverlayView;
import Core.Math;

export enum class StackAxis { Horizontal, Vertical };
export enum class CrossAlign { Start, Center, End };

// 자식을 한 줄로 나열하는 컨테이너. 자식의 앵커는 무시되고 위치는 스택이 정한다.
// 스택 크기는 고정이며, 내용이 넘쳐도 자르거나 줄이지 않는다.
export class Stack : public Widget
{
public:
    Stack(StackAxis axis, Core::Rect localRect)
        : Widget(localRect), m_axis(axis) {}

    void SetPadding(const Padding& padding) { m_padding = padding; InvalidateLayout(); }
    void SetSpacing(float spacing) { m_spacing = spacing; InvalidateLayout(); }
    void SetCrossAlign(CrossAlign align) { m_crossAlign = align; InvalidateLayout(); }

protected:
    void DrawSelf(IOverlayView&, const Core::Rect&) const override {} // 컨테이너는 그리지 않는다
    void ArrangeChildren() override;
    Core::Vector2 MeasureContent() const override;

private:
    StackAxis m_axis;
    Padding m_padding{};
    float m_spacing{ 0.f };
    CrossAlign m_crossAlign{ CrossAlign::Start };
};

// 사용 예: 메뉴 버튼 줄
//   auto& menu = ui.Attach(std::make_unique<Stack>(StackAxis::Vertical, Core::Rect{ 0, 0, 200, 300 }));
//   menu.SetSpacing(8.f);
//   menu.Attach(std::make_unique<Button>(...));