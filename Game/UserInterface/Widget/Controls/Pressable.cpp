module;

#include <nlohmann/json.hpp>

module UserInterface.Widget;

import std;
import :Pressable;
import Serialization;

bool SerializeButtonBrushes(Serializer& serializer, IWidgetResources& resources, ButtonBrushes& brushes, std::string_view prefix)
{
	return SerializeBrushName(serializer, resources, brushes.normal, SerializedKey(prefix, "normalBrush"))
		&& SerializeBrushName(serializer, resources, brushes.hover, SerializedKey(prefix, "hoverBrush"))
		&& SerializeBrushName(serializer, resources, brushes.pressed, SerializedKey(prefix, "pressedBrush"));
}
