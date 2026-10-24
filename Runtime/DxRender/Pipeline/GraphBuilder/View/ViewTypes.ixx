export module DxRender.Pipeline.GraphBuilder:ViewTypes;

import std;
import Core.Math;
import Contract.Render.Shared.View;
import DxRender.RGResourceID;

export struct ViewRenderOutput
{
	ViewID id;
	Core::Rect viewport;
	std::uint32_t heapIndex;
	RGResourceID colorID;
};