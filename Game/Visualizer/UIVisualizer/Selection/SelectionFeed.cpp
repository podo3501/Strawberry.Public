module;

#include <time.h>

module UIVisualizer.SelectionFeed;

import std;
import UIVisualizer.KeyQuery;
import UIVisualizer.SelectionReport;
import UIVisualizer.WidgetPathFinder;
import Contract.Input.Shared;
import Core.Logger;
import Core.Math;

namespace
{
    // 로컬 시각을 "2026-11-12 01:02:03" 형식으로 만든다. std::chrono::current_zone은 시간대 데이터베이스를 읽어 종료 때까지 남으므로 쓰지 않는다
    std::string LocalTimeText(std::chrono::system_clock::time_point now)
    {
        const std::time_t seconds = std::chrono::system_clock::to_time_t(now);
        std::tm local{};
        if (localtime_s(&local, &seconds) != 0)
        {
            Core::Logger::Error("로컬 시각을 구하지 못함");
            return "unknown";
        }

        return std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}",
            local.tm_year + 1900, local.tm_mon + 1, local.tm_mday, local.tm_hour, local.tm_min, local.tm_sec);
    }
}

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
        const Core::Vector2 point{ static_cast<float>(position.x), static_cast<float>(position.y) };
        if (!m_selection.Pick(m_layerRoot, point))   // 위젯이 없는 빈 곳(바닥)을 클릭하면 픽 창을 닫는다. 픽은 선택이 아니므로 파일은 그대로다
            m_selection.CancelPick();
    }

    // 휠을 위로 굴리면 안쪽(자식), 아래로 굴리면 바깥쪽(조상)으로 한 틱에 한 칸씩 옮긴다
    if (const int ticks = static_cast<int>(std::lround(input.GetMouseWheelDelta())); ticks != 0)
        m_selection.MovePick(-ticks);

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
    const std::string time = LocalTimeText(std::chrono::system_clock::now());

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
