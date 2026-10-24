export module Contract.Render.Shared.View:OverlayViewContext;

import std;
import :ViewID;
import :ViewTargetInfo;

export struct OverlayViewContext
{
	explicit OverlayViewContext(ViewID id) : target{ id } {}
	ViewTargetInfo target;
};