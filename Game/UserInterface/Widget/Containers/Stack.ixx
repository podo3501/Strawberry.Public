export module UserInterface.Widget:Stack;

import std;
import :Widget;
import Contract.Render.IOverlayView;
import Serialization;
import Core.Math;
import Core.TypeHierarchy;

export enum class StackAxis { Horizontal, Vertical, Count };   // Count는 저장용 이름 표(Stack.cpp)가 쓴다
export enum class CrossAlign { Start, Center, End, Count };

// 자식을 한 줄로 나열하는 컨테이너. 자식의 앵커는 무시되고 위치는 스택이 정한다.
// 크기는 기본이 Fixed이고, Fit으로 주면 자식 크기의 합(+간격, 패딩)에 맞춘다. 내용이 넘쳐도 자르지 않는다.
// 자식의 Fill 정책은 적용되지 않는다 (자식은 자기가 가진 크기를 그대로 쓴다).
export class Stack : public Core::TypeNode<Stack, Widget>
{
public:
    Stack(StackAxis axis, Core::Rect localRect)
        : TypeNode(localRect), m_axis(axis) {}

    void SetPadding(const Padding& padding) { m_padding = padding; InvalidateLayout(); }
    void SetSpacing(float spacing) { m_spacing = spacing; InvalidateLayout(); }
    void SetCrossAlign(CrossAlign align) { m_crossAlign = align; InvalidateLayout(); }

protected:
    bool Serialize(Serializer& serializer, IWidgetResources& resources) override;
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