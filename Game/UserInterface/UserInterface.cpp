module UserInterface;

import :UserInterface;

UserInterface::UserInterface(ITextMeasurer& measurer)
    : m_measurer(measurer)
{
    for (Group*& slot : m_layers)
    {
        auto& layer = m_root.Attach(std::make_unique<Group>());
        layer.SetSizePolicy(SizePolicy::Fill, SizePolicy::Fill);   // 화면 전체. Resize가 root를 바꾸면 따라온다
        slot = &layer;
    }
}

void UserInterface::Draw(IOverlayView& overlay) const
{
    m_root.Draw(overlay);
}

bool UserInterface::Update(const MouseState& mouse)
{
    const ButtonState& left = mouse[MouseButton::Left];

    m_pointer = Core::Vector2{ static_cast<float>(mouse.position.x), static_cast<float>(mouse.position.y) };

    if (left.IsPressed())
        m_tooltip.Dismiss();   // 누르면 툴팁을 끄고, 호버 대상이 바뀔 때까지 다시 띄우지 않는다

    const bool onUi = m_input.Update(m_root, PointerFrame{
        m_pointer,
        mouse.GetWheelDelta(),
        left.IsPressed(),
        left.IsReleased() });

    if (left.IsPressed())
        m_focus.OnPress(m_root, m_input.Pressed(m_root));   // 누름을 캡처한 위젯(없으면 포커스 해제)으로 포커스를 옮긴다

    return onUi;
}

bool UserInterface::OnKeyDown(KeyCode key, bool shift)
{
    return m_focus.OnKeyDown(m_root, KeyEvent{ key, shift });
}

bool UserInterface::OnTextInput(std::string_view utf8)
{
    return m_focus.OnText(m_root, utf8);
}

void UserInterface::Tick(float dt)
{
    m_focus.Tick(m_root, dt);
    const Core::Rect& screen = m_root.LocalRect();
    m_tooltip.Tick(dt, m_input.Hovered(m_root), m_pointer,
        Core::Vector2{ screen.width, screen.height }, LayerRoot(UILayer::Tooltip));
}

void UserInterface::Resize(float width, float height)
{
    m_root.SetSize(width, height);
}

void UserInterface::ConfigureTooltip(TooltipFactory factory, TooltipSettings settings)
{
    m_tooltip.Configure(std::move(factory), settings);
}