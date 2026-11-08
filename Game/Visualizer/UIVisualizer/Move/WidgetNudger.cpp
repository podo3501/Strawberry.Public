module UIVisualizer.WidgetNudger;

import std;
import UIVisualizer.KeyQuery;
import Contract.Input.Shared;

bool WidgetNudger::Update(float dt, const IInputService& input, std::span<Widget* const> selected)
{
    struct Arrow { KeyCode key; float x; float y; };
    static constexpr std::array arrows{ Arrow{ KeyCode::Left, -1.f, 0.f }, Arrow{ KeyCode::Right, 1.f, 0.f },
        Arrow{ KeyCode::Up, 0.f, -1.f }, Arrow{ KeyCode::Down, 0.f, 1.f } };

    const bool ctrl = KeyQuery::IsCtrlDown(input);   // Ctrl과 함께 누르는 키는 다른 기능의 몫이다
    const float step = KeyQuery::IsShiftDown(input) ? kBigStep : kStep;

    bool held = false;   // 화살표가 하나라도 눌려 있다
    Core::Vector2 delta{};
    for (const Arrow& arrow : arrows)
    {
        held |= input.GetKey(arrow.key).IsDown();
        if (!ctrl && KeyQuery::IsTriggered(input, arrow.key))
        {
            delta.x += arrow.x * step;
            delta.y += arrow.y * step;
        }
    }

    if ((delta.x != 0.f || delta.y != 0.f) && !selected.empty())
        Move(selected, delta);

    return Tick(dt, held);
}

bool WidgetNudger::Move(std::span<Widget* const> widgets, const Core::Vector2& delta)
{
    bool moved = false;
    for (Widget* widget : widgets)
    {
        const Core::Rect before = WidgetInspector::ScreenRect(*widget);
        const Core::Vector2 offset = WidgetInspector::GetAnchor(*widget).offset;

        widget->SetOffset(offset.x + delta.x, offset.y + delta.y);

        // 컨테이너가 배치하는 위젯은 오프셋을 바꿔도 제자리다. 그런 위젯은 되돌려 놓아 JSON에 헛된 값이 쓰이지 않게 한다
        const Core::Rect after = WidgetInspector::ScreenRect(*widget);
        if (after.x == before.x && after.y == before.y)
        {
            widget->SetOffset(offset.x, offset.y);
            continue;
        }
        moved = true;
    }

    if (moved)
    {
        m_pending = true;
        m_idle = 0.f;
    }
    return moved;
}

bool WidgetNudger::Tick(float dt, bool keyHeld)
{
    if (!m_pending)
        return false;

    if (keyHeld)
    {
        m_idle = 0.f;
        return false;
    }

    m_idle += dt;
    return m_idle >= kSettleTime;
}
