export module Game.UserInterface.Widget:Group;

import :Widget;
import Core.Math;

export class Group final : public Widget
{
public:
    Group() : Widget(Core::Rect{ 0.f, 0.f, 0.f, 0.f }) {}
    explicit Group(Core::Rect localRect) : Widget(localRect) {}

protected:
    void DrawSelf(IOverlayView&, const Core::Rect&) const override {}
};