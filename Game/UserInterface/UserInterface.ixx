export module Game.UserInterface:UserInterface;

import std;
import Game.UserInterface.Widget;
import Contract.Render.UI;
import Contract.Input.Service;

export class UserInterface
{
public:
    template<std::derived_from<Widget> T>
    T& Attach(std::unique_ptr<T> widget, std::size_t index = Widget::End)
    {
        return m_root.Attach(std::move(widget), index);
    }

    void Draw(IOverlayView& overlay) const;
    bool Update(const MouseState& mouse);

private:
    Widget* Resolve(const std::weak_ptr<Widget*>& handle) const;

    Group m_root;
    std::weak_ptr<Widget*> m_hovered;
    std::weak_ptr<Widget*> m_pressed;
};