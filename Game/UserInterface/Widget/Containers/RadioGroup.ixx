export module Game.UserInterface.Widget:RadioGroup;

import std;
import :Stack;
import :Toggle;
import Core.Math;

// 항목 중 하나만 켜지는 Toggle 묶음. 배치는 Stack이 정한다.
// 옵션은 AddOption으로 붙이며, 그룹이 옵션의 SetOnChanged를 가져가므로 옵션에 개별 콜백을 걸지 않는다.
// 선택된 항목을 다시 눌러도 꺼지지 않는다. SetSelected는 콜백을 부르지 않고, 사용자 조작으로 선택이 바뀔 때만 콜백이 불린다.
// 옵션을 Detach해서 다른 곳에 붙이면 그룹의 옵션에서 빠진다. 그 옵션을 그룹보다 오래 살려 두면 안 된다.
export class RadioGroup final : public Stack
{
public:
    RadioGroup(StackAxis axis, Core::Rect localRect) : Stack(axis, localRect) {}

    Toggle& AddOption(std::unique_ptr<Toggle> option)
    {
        const bool alreadySelected = Selected().has_value();

        Toggle& added = Attach(std::move(option));
        if (alreadySelected && added.IsChecked())
            added.SetChecked(false);   // 먼저 선택된 항목이 이긴다

        m_options.push_back(&added);
        added.SetOnChanged([this, &added](bool checked) { OnOptionChanged(added, checked); });
        return added;
    }

    std::optional<std::size_t> Selected() const
    {
        std::size_t index = 0;
        for (const Toggle* option : m_options)
        {
            if (!IsOption(option))
                continue;
            if (option->IsChecked())
                return index;
            ++index;
        }
        return std::nullopt;
    }

    void SetSelected(std::optional<std::size_t> index)   // nullopt = 모두 해제. 범위 밖이면 무시
    {
        if (index && *index >= OptionCount())
            return;

        std::size_t i = 0;
        for (Toggle* option : m_options)
        {
            if (!IsOption(option))
                continue;
            option->SetChecked(index && *index == i);
            ++i;
        }
    }

    void SetOnSelectionChanged(std::function<void(std::size_t)> onChanged) { m_onSelectionChanged = std::move(onChanged); }

private:
    // 포인터 비교만 한다 (Detach된 옵션은 역참조하지 않는다)
    bool IsOption(const Toggle* option) const
    {
        return std::ranges::any_of(Children(), [option](const auto& child) { return child.get() == option; });
    }

    std::size_t OptionCount() const
    {
        return static_cast<std::size_t>(std::ranges::count_if(m_options, [this](const Toggle* o) { return IsOption(o); }));
    }

    void OnOptionChanged(Toggle& changed, bool checked)
    {
        if (!IsOption(&changed))
            return;

        if (!checked)   // 선택된 항목을 다시 누름: 라디오는 꺼지지 않는다
        {
            changed.SetChecked(true);
            return;
        }

        std::size_t index = 0;
        bool found = false;
        for (Toggle* option : m_options)
        {
            if (!IsOption(option))
                continue;
            if (option == &changed)
                found = true;
            else
                option->SetChecked(false);
            if (!found)
                ++index;
        }

        if (m_onSelectionChanged)
        {
            auto onChanged = m_onSelectionChanged;   // 콜백이 그룹을 파괴해도 멤버를 더 건드리지 않도록 복사본으로 부른다
            onChanged(index);                        // 반드시 마지막에 부른다
        }
    }

    std::vector<Toggle*> m_options;
    std::function<void(std::size_t)> m_onSelectionChanged;
};
