module;

#include <nlohmann/json.hpp>

module UserInterface.Widget;

import std;
import :Image;
import Serialization;

bool Image::Serialize(Serializer& serializer, IWidgetResources& resources)
{
	if (!Widget::Serialize(serializer, resources))
		return false;

	if (!SerializeSlicedBrush(serializer, resources, m_brush, ""))
		return false;

	SerializeTint(serializer, m_tint, "hasTint", "tint");   // 흰색이 아닐 때만 적는다
	return true;
}
