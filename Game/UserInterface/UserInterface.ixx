export module Game.UserInterface:UserInterface;

import std;
import :IWidget;
import Contract.Render.UI;

export class UserInterface
{
public:
	template<std::derived_from<IWidget> T, typename... Args>
	T& Add(Args&&... args)
	{
		auto& ref = *m_widgets.emplace_back(
			std::make_unique<T>(std::forward<Args>(args)...));
		return static_cast<T&>(ref);
	}

	void Draw(IOverlayView& overlay) const
	{
		for (const auto& w : m_widgets)
			w->Draw(overlay);
	}

private:
	std::vector<std::unique_ptr<IWidget>> m_widgets;
};