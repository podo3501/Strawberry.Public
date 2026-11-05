export module UserInterface.Widget:RadioGroup;

import std;
import :Stack;
import :Toggle;
import Core.Math;
import Core.TypeHierarchy;

// 항목 중 하나만 켜지는 Toggle 묶음. 배치는 Stack이 정한다.
// 옵션은 AddOption으로 붙이며, 그룹이 옵션의 SetOnChanged를 가져가므로 옵션에 개별 콜백을 걸지 않는다.
// 선택된 항목을 다시 눌러도 꺼지지 않는다. SetSelected는 콜백을 부르지 않고, 사용자 조작으로 선택이 바뀔 때만 콜백이 불린다.
// 옵션을 Detach해서 다른 곳에 붙이면 그룹의 옵션에서 빠진다. 그 옵션을 그룹보다 오래 살려 두면 안 된다.
export class RadioGroup final : public Core::TypeNode<RadioGroup, Stack>
{
public:
    RadioGroup(StackAxis axis, Core::Rect localRect) : TypeNode(axis, localRect) {}

    Toggle& AddOption(std::unique_ptr<Toggle> option);
    std::optional<std::size_t> Selected() const;
    void SetSelected(std::optional<std::size_t> index);   // nullopt = 모두 해제. 범위 밖이면 무시
    void SetOnSelectionChanged(std::function<void(std::size_t)> onChanged) { m_onSelectionChanged = std::move(onChanged); }

protected:
    // 저장하는 것은 Stack의 배치 설정과 자식 Toggle들이다 (선택 상태는 각 Toggle의 checked). 선택 콜백은 저장하지 않는다.
    // 불러온 자식이 Toggle이면 옵션으로 등록한다. 켜진 Toggle이 여럿이면 앞의 것이 이긴다 (AddOption과 같은 규칙). Toggle이 아닌 자식은 그냥 자식으로 둔다
    void OnChildLoaded(Widget& child) override;

private:
    void Register(Toggle& added);   // 이미 붙어 있는 Toggle을 옵션으로 등록한다: 먼저 선택된 항목이 이기고, 선택 변화를 그룹이 가로챈다
    void OnOptionChanged(Toggle& changed, bool checked);

    // 포인터 비교만 한다 (Detach된 옵션은 역참조하지 않는다)
    bool IsOption(const Toggle* option) const
    {
        return std::ranges::any_of(Children(), [option](const auto& child) { return child.get() == option; });
    }

    std::size_t OptionCount() const
    {
        return static_cast<std::size_t>(std::ranges::count_if(m_options, [this](const Toggle* o) { return IsOption(o); }));
    }

    std::vector<Toggle*> m_options;
    std::function<void(std::size_t)> m_onSelectionChanged;
};
