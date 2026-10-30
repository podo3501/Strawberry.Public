module Game.UserInterface;

import :UserInterface;

void UserInterface::Draw(IOverlayView& overlay) const
{
    m_root.Draw(overlay);
}

// 파괴됐으면 nullptr, Detach돼서 트리 밖이어도 nullptr
Widget* UserInterface::Resolve(const std::weak_ptr<Widget*>& handle) const
{
    const auto token = handle.lock();
    Widget* w = token ? *token : nullptr;
    return (w && w->IsDescendantOf(m_root)) ? w : nullptr;
}

bool UserInterface::Update(const MouseState& mouse)
{
	const Core::Vector2 pos{ static_cast<float>(mouse.position.x), static_cast<float>(mouse.position.y) };
	const PointerEvent e{ pos };

	Widget* hit = m_root.HitTest(pos);

    // hover 전환
    Widget* hovered = Resolve(m_hovered);
    if (hit != hovered)
    {
        if (hovered) 
            hovered->OnPointerLeave();

        if (hit) 
            hit->OnPointerEnter();

        m_hovered = hit ? hit->Handle() : std::weak_ptr<Widget*>{};
    }

    if (const float wheelDelta = mouse.GetWheelDelta(); wheelDelta != 0.f)
    {
        const ScrollEvent scroll{ pos, wheelDelta };
        for (Widget* w = hit; w; w = w->Parent())
            if (w->OnScroll(scroll))
                break;
    }

    const ButtonState& left = mouse[MouseButton::Left];

    // 누름: hit에서 부모 쪽으로 올라가며 처음 true를 반환하는 위젯이 캡처
    if (left.IsPressed())
    {
        Widget* consumer = nullptr;
        for (Widget* w = hit; w && !consumer; w = w->Parent())
            if (w->OnPointerDown(e))
                consumer = w;

        m_pressed = consumer ? consumer->Handle() : std::weak_ptr<Widget*>{};
    }

    // 뗌: 캡처한 위젯에 Up 전달, 뗀 위치가 그 위젯(또는 자손) 위면 Click
    if (left.IsReleased())
    {
        if (Widget* pressed = Resolve(m_pressed))
        {
            const bool inside = hit && hit->IsDescendantOf(*pressed); // 콜백이 위젯을 파괴할 수 있으니 먼저 계산
            pressed->OnPointerUp(e);
            if (inside)
                pressed->OnClick(e);
        }
        m_pressed.reset();
    }

    return hit != nullptr || Resolve(m_pressed) != nullptr;
}

void UserInterface::Resize(float width, float height)
{
    m_root.SetSize(width, height);
}