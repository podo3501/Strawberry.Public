export module UserInterface.Widget:Widget;

import std;
import :LayoutTypes;
import :InputEvents;
import :WidgetResources;
import Contract.Render.IOverlayView;
import Serialization;
import Core.Math;
import Core.TypeHierarchy;

// 레이아웃 흐름 (구현: Widget.Layout.cpp)
//   무언가 바뀜(SetSize, SetAnchor, 내용 변화 등)
//   -> Fit이 아닌 가장 가까운 조상을 찾아서
//   -> MeasureTree : 아래->위. 자식을 먼저 재고, Fit 축만 MeasureContent()로 크기를 확정
//   -> ArrangeTree : 위->아래. ArrangeChildren()이 자식의 위치를 정함
// 크기는 정책(Fixed/Fit)이, 위치는 부모의 ArrangeChildren이 정한다.

export class InputRouter; //InputRouter가 Widget을 import 하니까 순환하지 않기 위해 전방선언. (export는 InputRouter가 클래스 선언시에 export로 했기 때문)
export class TooltipController;
export class FocusRouter;
export class WidgetInspector;
export class WidgetSerializer;

export class Widget : public Core::TypeRoot<Widget>
{
    friend class InputRouter;
    friend class TooltipController;
    friend class FocusRouter;
    friend class WidgetInspector;
    friend class WidgetSerializer;

public:
    virtual ~Widget() = default;

    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;

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

    // --- 이름 ---
    // 비어 있으면 이름 없음 (이름 없는 형제는 여러 개여도 된다). 비어 있지 않은 이름은 같은 부모 아래 형제끼리 겹치지 않는다.
    // '/'(경로 구분자)와 '#'(이름 없는 위젯의 임시 표기 "종류#순서")는 쓸 수 없다.
    // 규칙에 어긋나면(형제와 겹침, 금지 문자) 바꾸지 않고 false. Attach는 겹침을 막지 못하므로 저장/불러오기에서 한 번 더 검사한다.
    bool SetName(std::string name);

    // --- 툴팁 ---
    void SetTooltip(std::string text) { m_tooltip = std::move(text); }   // 비어 있으면 없음. 이 위젯이나 툴팁이 없는 히트 가능 자손 위에 포인터가 머물면 뜬다

    // --- 레이아웃 ---
    void SetAnchor(const Anchor& anchor);   // offset까지 통째로 교체됨
    void SetOffset(float x, float y);       // 앵커 기준 오프셋
    void SetSize(float width, float height);
    void SetSizePolicy(SizePolicy width, SizePolicy height);

    const Core::Rect& LocalRect() const { return m_rect; }
    
protected:
    explicit Widget(Core::Rect localRect);
    Widget(Core::Rect localRect, SizePolicy width, SizePolicy height);

    virtual void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const = 0; // 화면 좌표로 변환된 rect를 받아 자기 모습만 그림
    virtual void DrawAfterChildren(IOverlayView& overlay, const Core::Rect& screenRect) const {} // 자식 위에 덧그림 (스크롤 막대 등). 자식과 같은 클립 안에서 그려진다
    virtual void ArrangeChildren(); // 자식 배치 방식. 기본은 각 자식의 앵커로 배치. 스택 같은 컨테이너가 override
    virtual Core::Vector2 MeasureContent() const; // Fit 축의 크기를 정하는 내용 크기. 자식은 이미 측정된 뒤(아래->위)에 호출된다. 기본은 현재 크기를 그대로 돌려줘서 Fit이어도 변하지 않는다. 텍스트 같은 리프와 스택이 override
    virtual bool Serialize(Serializer& serializer, IWidgetResources& resources); // 저장/불러오기 공통 경로. 실패(리소스 이름을 모름 등)하면 false. 공통 레이아웃 속성을 처리하고, 파생 위젯은 Widget::Serialize를 먼저 부른 뒤 자기 속성을 더한다. 읽을 때는 키가 없으면 현재 값을 유지한다
    virtual void OnChildLoaded(Widget& child) {} // 저장된 UI를 불러올 때, 자식을 붙일 때마다 불린다. AddOption처럼 붙이는 것만으로는 안 되는 등록을 하는 컨테이너가 재정의한다 (RadioGroup). 자식의 종류는 Core::Cast로 알아본다
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

    // --- 키보드 (FocusRouter가 호출) ---
    virtual bool IsFocusable() const { return false; }   // Tab으로 포커스를 받는 위젯만 true
    virtual bool TrapsFocus() const { return false; }    // true면 Tab 이동과 키 입력이 이 위젯의 서브트리 안에 갇힌다 (모달)
    virtual void OnFocus(bool viaKeyboard) {}            // viaKeyboard: Tab으로 왔는가. 마우스로 받았으면 false (포커스 표시를 그리지 않는다)
    virtual void OnBlur() {}
    virtual bool OnKeyDown(const KeyEvent&) { return false; }   // true = 소비, false = 부모에게 버블. 포커스 위젯에서 시작한다
    virtual void OnTextInput(std::string_view utf8) {}          // 입력된 문자(UTF-8). 포커스 위젯만 받고 버블되지 않는다
    virtual void OnTick(float dt) {}                            // 시간 진행(초). 포커스 위젯만 받는다 (캐럿 깜빡임 등)
    
    // ArrangeChildren override용 도구
    const std::vector<std::unique_ptr<Widget>>& Children() const { return m_children; }
    Core::Vector2 Size() const { return { m_rect.width, m_rect.height }; }
    static void SetLocalPosition(Widget& w, const Core::Vector2& pos); // 배치 결과를 자식의 m_rect.x/y에 기록
    void InvalidateLayout(); // 내용/속성이 바뀌어 크기나 배치가 달라질 때 부른다

    // 입력 처리용 도구
    Core::Rect ScreenRect() const;

private:
    // 라우터 전용 (friend: InputRouter, FocusRouter, TooltipController). 클라이언트가 트리 구조나 히트 판정에 의존하지 못하게 닫아 둔다
    std::weak_ptr<Widget*> Handle();                    // 위젯이 죽으면 만료되는 추적용 핸들
    Widget* Parent() const { return m_parent; }
    Widget* HitTest(const Core::Vector2& screenPos);    // 맨 위(뒤에 Attach된) 히트 가능 위젯
    bool IsDescendantOf(const Widget& ancestor) const;  // 자기 자신 포함

    // 트리
    void AttachImpl(std::unique_ptr<Widget> child, std::size_t index);

    // 이름
    static bool IsValidName(std::string_view name);   // 금지 문자가 없는가 (빈 이름은 유효)
    bool HasSiblingNamed(std::string_view name) const;   // 나를 뺀 형제 중에 같은 이름이 있는가

    // 좌표 변환 (그리기와 hit test가 같은 계산을 쓴다)
    Core::Rect ScreenRectUnder(const Core::Vector2& parentOrigin) const;
    Core::Vector2 ParentScreenOrigin() const;
    void DrawTree(
        IOverlayView& overlay,
        const Core::Vector2& parentOrigin,
        const std::optional<Core::Rect>& clip) const;
    Widget* HitTestTree(const Core::Vector2& p, const Core::Vector2& parentOrigin);

    // TooltipController 전용 (friend)
    const std::string& Tooltip() const { return m_tooltip; }   

    // 레이아웃
    bool IsFit() const { return m_widthPolicy == SizePolicy::Fit || m_heightPolicy == SizePolicy::Fit; }
    void MeasureTree();        // 아래->위: 자식을 먼저 측정하고 Fit 축의 크기를 확정
    void ArrangeTree();        // 위->아래: ArrangeChildren 후 자식 서브트리로 재귀
    void Relayout();           // 내 크기/앵커가 바뀜: 부모가 형제까지 다시 배치

    static Widget* Resolve(const Widget& root, const std::weak_ptr<Widget*>& handle);   // 핸들이 가리키는 위젯. 파괴됐거나 root 트리 밖이면 nullptr

    Core::Rect m_rect; // 부모 기준 로컬 좌표(레이아웃이 계산한 결과)
    Anchor m_anchor;
    Widget* m_parent{ nullptr };
    std::vector<std::unique_ptr<Widget>> m_children;
    std::shared_ptr<Widget*> m_handle;   // 위젯이 죽으면 weak_ptr가 만료됨

    std::string m_name;
    std::string m_tooltip;
    SizePolicy m_widthPolicy{ SizePolicy::Fixed };
    SizePolicy m_heightPolicy{ SizePolicy::Fixed };
};