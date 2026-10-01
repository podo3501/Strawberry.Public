export module Client.Render.View:OverlayData;

import std;
import :ID;
import :OverlayContext;
import :OverlayDrawList;

export struct OverlayViewData
{
    OverlayViewContext context{ InvalidViewID };
    OverlayViewDrawList draws;
};