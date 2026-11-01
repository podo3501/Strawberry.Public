export module Game.UserInterface.Widget:Backdrop;

import std;
import :Widget;
import :LayoutTypes;
import :InputEvents;
import Contract.Render.Handles;
import Contract.Render.IOverlayView;
import Contract.Input.Shared;
import Core.Math;

// 모달/팝업 뒤를 막는 전체 크기 판. 대화상자나 메뉴는 이 위젯의 자식으로 붙인다.
// 판 위의 누름과 휠을 소비해서 아래 위젯에 닿지 않게 하고, 자식 바깥을 클릭하면 콜백을 부른다.
// dim 브러시를 주면 판을 그리고(모달), 주지 않으면 투명하다(드롭다운 같은 팝업).
// 부모 크기를 채우므로 레이어에 붙이면 화면 전체를 덮는다.
// 바깥 판정은 자식의 rect 기준이라, 자식 rect 밖으로 튀어나온 자손 위의 클릭은 바깥으로 본다.
// 모달이라 포커스를 가둔다 (Tab 이동이 자식 안에서만 돈다). Escape는 SetOnCancel 콜백을 부르고, 키는 모두 소비한다.
//
// 사용 예: 확인 대화상자
//   auto& modal = ui.Attach(std::make_unique<Backdrop>(dimBrush), UILayer::Popup);
//   auto& dialog = modal.Attach(std::make_unique<Group>(Core::Rect{ 0, 0, 300, 160 }));
//   dialog.SetAnchor(Anchor::Center());
//   modal.SetOnOutsideClick([&] { modal.Detach(); });   // 닫기. 바깥 클릭으로 안 닫으려면 콜백을 비워 둔다
export class Backdrop final : public Widget
{
public:
    explicit Backdrop(std::optional<BrushHandle> dim = std::nullopt)
        : Widget(Core::Rect{ 0.f, 0.f, 0.f, 0.f }), m_dim(dim)
    {
        SetSizePolicy(SizePolicy::Fill, SizePolicy::Fill);
    }

    void SetOnOutsideClick(std::function<void()> onOutsideClick) { m_onOutsideClick = std::move(onOutsideClick); }
    void SetOnCancel(std::function<void()> onCancel) { m_onCancel = std::move(onCancel); }   // Escape

protected:
    bool IsHitTestable() const override { return true; }
    bool OnPointerDown(const PointerEvent&) override { return true; }   // 캡처: 아래 위젯으로 버블되지 않는다
    bool OnScroll(const ScrollEvent&) override { return true; }         // 휠도 아래로 새지 않는다
    bool TrapsFocus() const override { return true; }

    bool OnKeyDown(const KeyEvent& e) override
    {
        if (e.key == KeyCode::Escape && m_onCancel)
        {
            auto callback = m_onCancel;   // 콜백이 이 위젯을 파괴(Detach)해도 실행 중인 함수 객체는 살아 있어야 한다
            callback();
        }
        return true;   // 모달: 모든 키를 소비해서 게임 쪽으로 새지 않게 한다 (Tab은 FocusRouter가 먼저 처리한다)
    }

    void OnClick(const PointerEvent& e) override
    {
        if (!m_onOutsideClick || OverChild(e.screenPos))
            return;

        auto callback = m_onOutsideClick;   // 콜백이 이 위젯을 파괴(Detach)해도 실행 중인 함수 객체는 살아 있어야 한다
        callback();
    }

    void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const override
    {
        if (m_dim)
            overlay.DrawImage(*m_dim, screenRect);
    }

private:
    bool OverChild(const Core::Vector2& screenPos) const
    {
        const Core::Rect self = ScreenRect();
        const float x = screenPos.x - self.x;
        const float y = screenPos.y - self.y;

        for (const auto& child : Children())
            if (child->LocalRect().Contains(x, y))
                return true;
        return false;
    }

    std::optional<BrushHandle> m_dim;
    std::function<void()> m_onOutsideClick;
    std::function<void()> m_onCancel;
};