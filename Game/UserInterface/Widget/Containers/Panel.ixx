export module UserInterface.Widget:Panel;

import std;
import :Stack;
import :SlicedBrush;
import :WidgetResources;
import Contract.Render.IOverlayView;
import Serialization;
import Core.Math;
import Core.TypeHierarchy;

// 배경(9슬라이스 가능)을 깔고 자식을 한 줄로 나열하는 컨테이너. 대화상자, 메뉴 창, 툴팁 말풍선에 쓴다.
// 나열 방식(패딩, 간격, 교차축 정렬, Fit 크기)은 Stack과 같고, 자식의 앵커는 무시된다.
// 테두리가 있는 스킨이면 패딩을 테두리 두께 이상으로 줘야 내용이 테두리 위에 그려지지 않는다.
// 기본은 입력을 통과시킨다 (툴팁처럼 마우스를 가리면 안 되는 용도). 뒤쪽 위젯으로 입력이 새면 안 되는 창이면 SetBlocksInput(true).
export class Panel final : public Core::TypeNode<Panel, Stack>
{
public:
    Panel(StackAxis axis, Core::Rect localRect, SlicedBrush background)
        : TypeNode(axis, localRect), m_background(background) {}

    void SetBlocksInput(bool blocks) { m_blocksInput = blocks; }
    void SetBackgroundTint(const Core::Color& tint) { m_backgroundTint = tint; }   // 배경 이미지에 곱하는 색. 기본은 흰색(그대로). 흰색 이미지를 늘려 단색 배경으로 쓸 때 색을 준다

protected:
    bool Serialize(Serializer& serializer, IWidgetResources& resources) override;   // 구현: Panel.cpp
    bool IsHitTestable() const override { return m_blocksInput; }

    void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const override
    {
        if (screenRect.width > 0.f && screenRect.height > 0.f)   // 빈 Fit 패널은 그릴 것이 없다
            DrawSliced(overlay, m_background, screenRect, m_backgroundTint);
    }

private:
    SlicedBrush m_background;
    Core::Color m_backgroundTint{};
    bool m_blocksInput{ false };
};

// 사용 예: 확인 대화상자 (Backdrop 위에 붙인다)
//   auto& dialog = modal.Attach(std::make_unique<Panel>(StackAxis::Vertical, Core::Rect{ 0, 0, 0, 0 },
//       SlicedBrush::Sliced(frame, SliceMargins::Uniform(8.f), Core::Vector2{ 32.f, 32.f })));
//   dialog.SetSizePolicy(SizePolicy::Fit, SizePolicy::Fit);
//   dialog.SetPadding(Padding::All(12.f));
//   dialog.SetSpacing(8.f);
//   dialog.SetBlocksInput(true);