module;

#include <nlohmann/json.hpp>

module UserInterface.Widget;

import std;
import :Backdrop;
import Serialization;

// dim 브러시가 없으면(투명 판) hasDim만 false로 적고 브러시 키는 적지 않는다. 콜백 3개는 저장하지 않는다
bool Backdrop::Serialize(Serializer& serializer, IWidgetResources& resources)
{
	if (!Widget::Serialize(serializer, resources))
		return false;

	bool hasDim = m_dim.has_value();
	BrushHandle dim = m_dim.value_or(BrushHandle{});
	serializer.Process("hasDim", hasDim);
	if (hasDim && !SerializeBrushName(serializer, resources, dim, "dimBrush"))
		return false;

	if (!serializer.IsWrite())   // 저장할 때는 객체를 건드리지 않는다
		m_dim = hasDim ? std::optional<BrushHandle>(dim) : std::nullopt;
	return true;
}
