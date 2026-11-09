module;

#include <nlohmann/json.hpp>

module UserInterface.Widget;

import std;
import :Pressable;
import :Slice;
import :SlicedBrush;
import Serialization;
import Core.Math;

namespace
{
	bool SerializeState(Serializer& serializer, IWidgetResources& resources, StateBrush& state, std::string_view prefix,
		std::string_view brushKey, std::string_view hasTintKey, std::string_view tintKey)
	{
		if (!SerializeBrushName(serializer, resources, state.brush, SerializedKey(prefix, brushKey)))
			return false;

		SerializeTint(serializer, state.tint, SerializedKey(prefix, hasTintKey), SerializedKey(prefix, tintKey));
		return true;
	}

	// 9슬라이스일 때만 적는다. 읽을 때 키가 없으면 통짜 이미지로 돌아간다
	void SerializeSlice(Serializer& serializer, ButtonBrushes& brushes, std::string_view prefix)
	{
		bool sliced = serializer.IsWrite() && brushes.margins.has_value();
		if (serializer.IsWrite() && !sliced)
			return;

		serializer.Process(SerializedKey(prefix, "sliced"), sliced);
		if (!sliced)
		{
			brushes.margins.reset();
			brushes.srcSize = { 1.f, 1.f };
			return;
		}

		SliceMargins margins = brushes.margins.value_or(SliceMargins{});
		SerializeSliceMargins(serializer, margins, brushes.srcSize, prefix);
		brushes.margins = margins;
	}
}

bool SerializeButtonBrushes(Serializer& serializer, IWidgetResources& resources, ButtonBrushes& brushes, std::string_view prefix)
{
	if (!SerializeState(serializer, resources, brushes.normal, prefix, "normalBrush", "hasNormalTint", "normalTint")
		|| !SerializeState(serializer, resources, brushes.hover, prefix, "hoverBrush", "hasHoverTint", "hoverTint")
		|| !SerializeState(serializer, resources, brushes.pressed, prefix, "pressedBrush", "hasPressedTint", "pressedTint"))
		return false;

	SerializeSlice(serializer, brushes, prefix);
	return true;
}
