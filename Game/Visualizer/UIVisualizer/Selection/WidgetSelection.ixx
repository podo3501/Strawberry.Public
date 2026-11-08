export module UIVisualizer.WidgetSelection;

import std;
import UserInterface;
import Core.Math;

// 위젯을 고르는 두 단계, 픽(후보)과 선택 (구현: WidgetSelection.cpp)
// - 픽: 화면을 클릭하면 그 자리의 위젯 체인(가장 안쪽 위젯부터 바깥쪽 조상까지, 레이어 루트 제외)에서 후보 하나를 가리킨다.
//   같은 위젯 위를 계속 클릭하면 안쪽 → 바깥쪽으로 한 칸씩 올라가고, 맨 위에서 한 번 더 클릭하면 가장 안쪽으로 돌아온다. 다른 위젯 위를 클릭하면 그 위젯의 가장 안쪽부터 새로 시작한다.
//   픽은 선택이 아니다. 화면에 후보를 보여 주는 용도이고, 옮기기나 Claude에게 알리기의 대상이 되지 않는다.
// - 선택: SelectPicked로 지금 픽한 위젯을 선택 목록에 넣는다 (이미 있으면 뺀다). 고른 순서가 곧 번호다(순서 + 1). 선택하면 픽은 끝난다.
// 위젯을 가리키는 포인터만 들고 있으므로, 레이아웃을 다시 읽어 위젯이 새로 만들어지면 Clear로 모두 비워야 한다.
export class WidgetSelection
{
public:
    static constexpr std::size_t kPickWindowSize = 5;   // 픽을 보여 줄 때 한 번에 보이는 체인의 위젯 수

    // layerRoot 아래에서 point(화면 좌표)에 보이는 가장 위의 위젯을 찾아 그 체인에서 픽한다 (위의 규칙).
    // 위젯은 깊이 들어간 쪽(자식)과 나중에 그려지는 쪽(나중에 붙인 형제)이 위다. 입력에 반응하지 않는 위젯(이미지, 글자 등)도 고른다. 레이어 루트 자신은 고르지 않는다.
    // 위젯이 없는 곳이면 아무것도 바꾸지 않고 false
    bool Pick(Widget& layerRoot, const Core::Vector2& point);

    // 픽 창을 띄울 기준 위치. 새로 시작한 클릭(다른 위젯을 클릭했을 때)의 위치이고, 같은 곳을 계속 클릭해 올라가는 동안에는 그대로다 (창이 움직이지 않게 한다)
    const Core::Vector2& PickAnchor() const noexcept { return m_anchor; }

    bool HasPick() const noexcept { return !m_chain.empty(); }
    const Widget* Picked() const noexcept { return m_chain.empty() ? nullptr : m_chain[m_pickIndex]; }   // 지금 픽한 위젯. 없으면 nullptr

    // 화면에 보여 줄 체인의 일부: 최대 kPickWindowSize개이고 Picked를 포함한다 (안쪽부터 바깥쪽 순서).
    // Picked를 가운데에 두되, 체인의 처음과 끝 가까이에서는 창이 체인 밖으로 나가지 않아 Picked가 창의 가장자리로 움직인다. 체인이 짧으면 전부
    std::span<Widget* const> PickWindow() const noexcept;

    void CancelPick() noexcept { m_chain.clear(); m_pickIndex = 0; }

    // 지금 픽한 위젯을 선택 목록 끝에 넣고(이미 있으면 뺀다) 픽을 끝낸다. 픽이 없으면 아무것도 하지 않고 false
    bool SelectPicked();

    // 클릭 없이 위젯을 목록 끝에 넣는다 (다시 읽은 뒤 선택을 되살릴 때 쓴다). 이미 있으면 아무것도 하지 않는다
    void Add(Widget& widget)
    {
        if (std::ranges::find(m_items, &widget) == m_items.end())
            m_items.push_back(&widget);
    }

    void Clear() noexcept { m_items.clear(); CancelPick(); }   // 선택과 픽을 모두 비운다

    // 고른 순서대로. 번호는 인덱스 + 1
    // 고른 위젯은 키보드로 옮기는 등 값을 바꾸는 데도 쓰므로 const가 아니다. 읽기만 하는 곳은 span<const Widget* const>로 받을 수 있다
    std::span<Widget* const> Items() const { return m_items; }

private:
    std::vector<Widget*> m_items;
    std::vector<Widget*> m_chain;   // 픽 중인 체인 (안쪽부터). 픽이 없으면 비어 있다
    std::size_t m_pickIndex{ 0 };   // m_chain 안에서 지금 픽한 위치
    Core::Vector2 m_anchor{};       // 픽 창의 기준 위치
};
