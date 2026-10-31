module Game.UserInterface;

import :UserInterface;

void UserInterface::Draw(IOverlayView& overlay) const
{
    m_root.Draw(overlay);
}

bool UserInterface::Update(const MouseState& mouse)
{
    const ButtonState& left = mouse[MouseButton::Left];

    return m_input.Update(m_root, PointerFrame{
        Core::Vector2{ static_cast<float>(mouse.position.x), static_cast<float>(mouse.position.y) },
        mouse.GetWheelDelta(),
        left.IsPressed(),
        left.IsReleased() });
}

void UserInterface::Resize(float width, float height)
{
    m_root.SetSize(width, height);
}