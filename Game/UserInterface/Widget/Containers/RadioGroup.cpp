module UserInterface.Widget;

import std;
import :RadioGroup;
import Core.TypeHierarchy;

Toggle& RadioGroup::AddOption(std::unique_ptr<Toggle> option)
{
    Toggle& added = Attach(std::move(option));
    Register(added);
    return added;
}

std::optional<std::size_t> RadioGroup::Selected() const
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

void RadioGroup::SetSelected(std::optional<std::size_t> index)
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

void RadioGroup::OnChildLoaded(Widget& child)
{
    if (Toggle* toggle = Core::Cast<Toggle>(&child))
        Register(*toggle);
}

void RadioGroup::Register(Toggle& added)
{
    const bool alreadySelected = Selected().has_value();
    if (alreadySelected && added.IsChecked())
        added.SetChecked(false);   // 먼저 선택된 항목이 이긴다

    m_options.push_back(&added);
    added.SetOnChanged([this, &added](bool checked) { OnOptionChanged(added, checked); });
}

void RadioGroup::OnOptionChanged(Toggle& changed, bool checked)
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
