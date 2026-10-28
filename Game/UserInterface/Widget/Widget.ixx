export module Game.UserInterface.Widget:Widget;

import std;
import Contract.Render.IOverlayView;
import Core.Math;

export struct PointerEvent
{
    Core::Vector2 screenPos;
};

export struct Widget
{
public:
    virtual ~Widget() = default;

    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;

    static constexpr std::size_t End = (std::numeric_limits<std::size_t>::max)();

    virtual bool IsHitTestable() const { return false; } // 입력을 받는 위젯만 true
    virtual bool OnPointerDown(const PointerEvent&) { return false; } // true = 소비(캡처), false = 부모에게 버블
    virtual void OnPointerUp(const PointerEvent&) {}
    virtual void OnClick(const PointerEvent&) {}
    virtual void OnPointerEnter() {}
    virtual void OnPointerLeave() {}
    
    template<std::derived_from<Widget> T>
    T& Attach(std::unique_ptr<T> child, std::size_t index = End) // 소유권을 넘겨받아 붙임. index 위치에 삽입되며 인덱스 순서 = 그리기 순서(뒤가 위)
    {
        T& ref = *child;
        AttachImpl(std::move(child), index);
        return ref;
    }
    
    std::unique_ptr<Widget> Detach(); // 부모에게서 떼어내 소유권을 돌려줌. 받은 쪽이 버리면 파괴, 다른 위젯에 Attach하면 이동
    void Draw(IOverlayView& overlay) const;
    Widget* HitTest(const Core::Vector2& screenPos);                  // 맨 위(뒤에 Attach된) 히트 가능 위젯

    std::weak_ptr<Widget*> Handle() const { return m_handle; }
    void SetPosition(float x, float y) { m_rect.x = x; m_rect.y = y; }
    const Core::Rect& LocalRect() const { return m_rect; }
    Widget* Parent() const { return m_parent; }
    
protected:
    explicit Widget(Core::Rect localRect) : m_rect(localRect) {}
    virtual void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const = 0; // 화면 좌표로 변환된 rect를 받아 자기 모습만 그림

private:
    Core::Vector2 ParentScreenOrigin() const;
    void AttachImpl(std::unique_ptr<Widget> child, std::size_t index);
    bool IsDescendantOf(const Widget& ancestor) const;
    void DrawTree(IOverlayView& overlay, const Core::Vector2& origin) const;
    Widget* HitTestTree(const Core::Vector2& p, const Core::Vector2& parentOrigin); // origin 누적 방식은 DrawTree와 같아야 함

    Core::Rect m_rect; // 부모 기준 로컬 좌표
    Widget* m_parent{ nullptr };
    std::vector<std::unique_ptr<Widget>> m_children;
    std::shared_ptr<Widget*> m_handle{ std::make_shared<Widget*>(this) }; // 위젯이 죽으면 weak_ptr가 만료됨
};