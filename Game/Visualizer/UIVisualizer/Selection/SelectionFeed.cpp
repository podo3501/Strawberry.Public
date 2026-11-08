module UIVisualizer.SelectionFeed;

import std;
import UIVisualizer.KeyQuery;
import UIVisualizer.SelectionReport;
import UIVisualizer.WidgetPathFinder;
import Contract.Input.Shared;
import Core.Logger;
import Core.Math;

SelectionFeed::SelectionFeed(Widget& layerRoot, IAssetStorage& storage, ITextMeasurer& measurer, std::string layoutLabel)
    : m_layerRoot{ layerRoot }
    , m_storage{ storage }
    , m_layoutLabel{ std::move(layoutLabel) }
{
    m_overlay.SetMeasurer(measurer);
}

void SelectionFeed::HandleInput(const IInputService& input)
{
    if (input.GetKey(KeyCode::Escape).IsPressed())
    {
        if (m_selection.HasPick())
            m_selection.CancelPick();
        else
            Clear();
    }

    if (input.GetMouseButton(MouseButton::Left).IsPressed())
    {
        const Core::Point position = input.GetMousePosition();
        m_selection.Pick(m_layerRoot, Core::Vector2{ static_cast<float>(position.x), static_cast<float>(position.y) });   // 픽은 선택이 아니므로 파일은 그대로다
    }

    if (input.GetKey(KeyCode::C).IsPressed() && !KeyQuery::IsCtrlDown(input) && m_selection.SelectPicked())
        Write();
}

void SelectionFeed::CheckHandoff(float dt)
{
    if (!m_selection.Items().empty() && m_handoff.WasTaken(dt, m_storage, kFile))
        Clear();
}

void SelectionFeed::Clear()
{
    const bool hadSelection = !m_selection.Items().empty();
    m_selection.Clear();   // 픽도 함께 비운다
    if (hadSelection)
        Write();
}

bool SelectionFeed::Write()
{
    const auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    const std::string time = std::format("{:%F %T}", std::chrono::zoned_time{ std::chrono::current_zone(), now });

    const std::string text = FormatSelection(m_layerRoot, m_selection.Items(), m_layoutLabel, time);
    if (!m_storage.WriteText(kFile, text))
    {
        Core::Logger::Error("선택 정보 파일을 쓰지 못함: {}", kFile.String());
        return false;
    }
    return true;
}

std::vector<std::string> SelectionFeed::Paths() const
{
    std::vector<std::string> paths;
    for (const Widget* widget : m_selection.Items())
        paths.push_back(WidgetPath(m_layerRoot, *widget));
    return paths;
}

void SelectionFeed::Restore(std::span<const std::string> paths)
{
    for (const std::string& path : paths)
        if (Widget* widget = FindWidgetByPath(m_layerRoot, path))
            m_selection.Add(*widget);

    Write();   // 다시 읽는 동안 비워 둔 선택 정보 파일을 복원된 선택으로 쓴다
}
