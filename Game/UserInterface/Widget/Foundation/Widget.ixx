export module Game.UserInterface.Widget:Widget;

import std;
import :LayoutTypes;
import :InputEvents;
import Contract.Render.IOverlayView;
import Core.Math;

// 레이아웃 흐름 (구현: Widget.Layout.cpp)
//   무언가 바뀜(SetSize, SetAnchor, 내용 변화 등)
//   -> Fit이 아닌 가장 가까운 조상을 찾아서
//   -> MeasureTree : 아래->위. 자식을 먼저 재고, Fit 축만 MeasureContent()로 크기를 확정
//   -> ArrangeTree : 위->아래. ArrangeChildren()이 자식의 위치를 정함
// 크기는 정책(Fixed/Fit)이, 위치는 부모의 ArrangeChildren이 정한다.

export class InputRouter; //InputRouter가 Widget을 import 하니까 순환하지 않기 위해 전방선언. (export는 InputRouter가 클래스 선언시에 export로 했기 때문)

export class Widget
{
    friend class InputRouter;

public:
    virtual ~Widget() = default;

    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;

    std::weak_ptr<Widget*> Handle();
    void Draw(IOverlayView& overlay) const;

    // -- Tree ---
    template<std::derived_from<Widget> T>
    T& Attach(std::unique_ptr<T> child) // 맨 뒤(맨 위)에 붙임
    {
        return Attach(std::move(child), m_children.size());
    }

    template<std::derived_from<Widget> T>
    T& Attach(std::unique_ptr<T> child, std::size_t index) // index 위치에 삽입. size를 넘으면 맨 뒤
    {
        T& ref = *child;
        AttachImpl(std::move(child), index);
        return ref;
    }

    std::unique_ptr<Widget> Detach(); // 부모에게서 떼어내 소유권을 돌려줌. 받은 쪽이 버리면 파괴, 다른 위젯에 Attach하면 이동
    bool IsDescendantOf(const Widget& ancestor) const; // 자기 자신 포함

    // --- 레이아웃 ---
    void SetAnchor(const Anchor& anchor);   // offset까지 통째로 교체됨
    void SetOffset(float x, float y);       // 앵커 기준 오프셋 (구 SetPosition)
    void SetSize(float width, float height);
    void SetSizePolicy(SizePolicy width, SizePolicy height);
    const Anchor& GetAnchor() const { return m_anchor; }

    const Core::Rect& LocalRect() const { return m_rect; }
    Widget* Parent() const { return m_parent; }
    Widget* HitTest(const Core::Vector2& screenPos); // 맨 위(뒤에 Attach된) 히트 가능 위젯
    
protected:
    explicit Widget(Core::Rect localRect);
    virtual void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const = 0; // 화면 좌표로 변환된 rect를 받아 자기 모습만 그림
    virtual void ArrangeChildren(); // 자식 배치 방식. 기본은 각 자식의 앵커로 배치. 스택 같은 컨테이너가 override
    virtual Core::Vector2 MeasureContent() const; // Fit 축의 크기를 정하는 내용 크기. 자식은 이미 측정된 뒤(아래->위)에 호출된다. 기본은 현재 크기를 그대로 돌려줘서 Fit이어도 변하지 않는다. 텍스트 같은 리프와 스택이 override
    virtual bool ClipsChildren() const { return false; } // true면 자식(과 그 자손)을 내 rect로 자른다. 잘린 영역은 히트 테스트에서도 제외된다

    // --- 입력 (InputRouter가 호출) ---
    virtual bool IsHitTestable() const { return false; } // 입력을 받는 위젯만 true
    virtual bool OnPointerDown(const PointerEvent&) { return false; } // true = 소비(캡처), false = 부모에게 버블
    virtual void OnPointerMove(const PointerEvent&) {}   // 캡처한 위젯만 받는다 (누른 채 움직일 때)
    virtual void OnPointerUp(const PointerEvent&) {}
    virtual void OnClick(const PointerEvent&) {}
    virtual bool OnScroll(const ScrollEvent&) { return false; } // true = 소비, false = 부모에게 버블
    virtual void OnPointerEnter() {}
    virtual void OnPointerLeave() {}
    
    // ArrangeChildren override용 도구
    const std::vector<std::unique_ptr<Widget>>& Children() const { return m_children; }
    Core::Vector2 Size() const { return { m_rect.width, m_rect.height }; }
    static void SetLocalPosition(Widget& w, const Core::Vector2& pos); // 배치 결과를 자식의 m_rect.x/y에 기록

    // 입력 처리용 도구
    Core::Rect ScreenRect() const;

    void InvalidateLayout();

private:
    // 트리
    void AttachImpl(std::unique_ptr<Widget> child, std::size_t index);

    // 좌표 변환 (그리기와 hit test가 같은 계산을 쓴다)
    Core::Rect ScreenRectUnder(const Core::Vector2& parentOrigin) const;
    Core::Vector2 ParentScreenOrigin() const;
    void DrawTree(IOverlayView& overlay, const Core::Vector2& parentOrigin) const;
    Widget* HitTestTree(const Core::Vector2& p, const Core::Vector2& parentOrigin);

    // 레이아웃
    bool IsFit() const { return m_widthPolicy == SizePolicy::Fit || m_heightPolicy == SizePolicy::Fit; }
    void MeasureTree();        // 아래->위: 자식을 먼저 측정하고 Fit 축의 크기를 확정
    void ArrangeTree();        // 위->아래: ArrangeChildren 후 자식 서브트리로 재귀
    void Relayout();           // 내 크기/앵커가 바뀜: 부모가 형제까지 다시 배치

    Core::Rect m_rect; // 부모 기준 로컬 좌표(레이아웃이 계산한 결과)
    Anchor m_anchor;
    Widget* m_parent{ nullptr };
    std::vector<std::unique_ptr<Widget>> m_children;
    std::shared_ptr<Widget*> m_handle;   // 위젯이 죽으면 weak_ptr가 만료됨

    SizePolicy m_widthPolicy{ SizePolicy::Fixed };
    SizePolicy m_heightPolicy{ SizePolicy::Fixed };
};