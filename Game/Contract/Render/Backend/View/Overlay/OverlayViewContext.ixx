export module Contract.Render.Backend.View:OverlayContext;

import std;
import :ID;
import :TargetInfo;

export struct OverlayViewContext
{
    explicit OverlayViewContext(ViewID id) : target{ id } {}
    ViewTargetInfo target;
};