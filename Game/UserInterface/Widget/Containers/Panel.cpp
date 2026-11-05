module;

#include <nlohmann/json.hpp>

module UserInterface.Widget;

import std;
import :Panel;
import Serialization;

bool Panel::Serialize(Serializer& serializer, IWidgetResources& resources)
{
	if (!Stack::Serialize(serializer, resources))
		return false;

	if (!SerializeSlicedBrush(serializer, resources, m_background, "background"))
		return false;

	serializer.Process("blocksInput", m_blocksInput);
	return true;
}
