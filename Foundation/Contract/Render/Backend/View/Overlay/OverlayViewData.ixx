export module Contract.Render.Backend.View:OverlayData;

import std;
import :OverlayDrawList;
import Contract.Render.Shared.View;

export struct OverlayViewData
{
    OverlayViewContext context{ InvalidViewID };
    OverlayViewDrawList draws;
};