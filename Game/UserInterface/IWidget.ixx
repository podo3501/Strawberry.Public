export module Game.UserInterface:IWidget;

import Contract.Render.IOverlayView;

export struct IWidget
{
	virtual ~IWidget() = default;
	virtual void Draw(IOverlayView& overlay) const = 0;
};