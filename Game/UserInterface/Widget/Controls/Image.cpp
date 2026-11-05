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

	return SerializeSlicedBrush(serializer, resources, m_brush, "");
}
