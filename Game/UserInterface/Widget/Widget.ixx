export module Game.UserInterface.Widget:Widget;

import std;
import Contract.Render.IOverlayView;
import Core.Math;

export struct Widget
{
public:
    virtual ~Widget() = default;

    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;

    static constexpr std::size_t End = (std::numeric_limits<std::size_t>::max)();
    
    template<std::derived_from<Widget> T>
    T& Attach(std::unique_ptr<T> child, std::size_t index = End) // 소유권을 넘겨받아 붙임. index 위치에 삽입되며 인덱스 순서 = 그리기 순서(뒤가 위)
    {
        T& ref = *child;
        AttachImpl(std::move(child), index);
        return ref;
    }
    
    std::unique_ptr<Widget> Detach(); // 부모에게서 떼어내 소유권을 돌려줌. 받은 쪽이 버리면 파괴, 다른 위젯에 Attach하면 이동
    void Draw(IOverlayView& overlay) const;

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

    Core::Rect m_rect; // 부모 기준 로컬 좌표
    Widget* m_parent{ nullptr };
    std::vector<std::unique_ptr<Widget>> m_children;
};