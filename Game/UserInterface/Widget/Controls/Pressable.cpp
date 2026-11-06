module;

#include <nlohmann/json.hpp>

module UserInterface.Widget;

import std;
import :Pressable;
import Serialization;
import Core.Math;

namespace
{
	// 흰색(기본)이 아닐 때만 적는다. 읽을 때 키가 없으면 흰색으로 돌아간다
	void SerializeTint(Serializer& serializer, Core::Color& tint, const std::string& hasKey, const std::string& colorKey)
	{
		bool has = tint != Core::Color{};
		if (serializer.IsWrite() && !has)
			return;

		serializer.Process(hasKey, has);
		if (!has)
		{
			tint = Core::Color{};
			return;
		}

		serializer.Process(colorKey + "R", tint.r);
		serializer.Process(colorKey + "G", tint.g);
		serializer.Process(colorKey + "B", tint.b);
		serializer.Process(colorKey + "A", tint.a);
	}

	bool SerializeState(Serializer& serializer, IWidgetResources& resources, StateBrush& state, std::string_view prefix,
		std::string_view brushKey, std::string_view hasTintKey, std::string_view tintKey)
	{
		if (!SerializeBrushName(serializer, resources, state.brush, SerializedKey(prefix, brushKey)))
			return false;

		SerializeTint(serializer, state.tint, SerializedKey(prefix, hasTintKey), SerializedKey(prefix, tintKey));
		return true;
	}
}

bool SerializeButtonBrushes(Serializer& serializer, IWidgetResources& resources, ButtonBrushes& brushes, std::string_view prefix)
{
	return SerializeState(serializer, resources, brushes.normal, prefix, "normalBrush", "hasNormalTint", "normalTint")
		&& SerializeState(serializer, resources, brushes.hover, prefix, "hoverBrush", "hasHoverTint", "hoverTint")
		&& SerializeState(serializer, resources, brushes.pressed, prefix, "pressedBrush", "hasPressedTint", "pressedTint");
}
