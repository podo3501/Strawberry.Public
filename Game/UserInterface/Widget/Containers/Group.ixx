export module UserInterface.Widget:Group;

import std;
import :Widget;
import Contract.Render.IOverlayView;
import Core.Math;
import Core.TypeHierarchy;

export class Group final : public Core::TypeNode<Group, Widget>
{
public:
    Group() : TypeNode(Core::Rect{ 0.f, 0.f, 0.f, 0.f }) {}
    explicit Group(Core::Rect localRect) : TypeNode(localRect) {}

protected:
    void DrawSelf(IOverlayView&, const Core::Rect&) const override {}
};