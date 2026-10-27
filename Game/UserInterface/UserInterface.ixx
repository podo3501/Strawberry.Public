export module Game.UserInterface:UserInterface;

import std;
import Game.UserInterface.Widget;
import Contract.Render.UI;

export class UserInterface
{
public:
    template<std::derived_from<Widget> T>
    T& Attach(std::unique_ptr<T> widget, std::size_t index = Widget::End)
    {
        return m_root.Attach(std::move(widget), index);
    }

    void Draw(IOverlayView& overlay) const
    {
        m_root.Draw(overlay);
    }

private:
    Group m_root;
};