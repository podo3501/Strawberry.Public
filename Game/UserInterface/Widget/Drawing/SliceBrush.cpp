module;

#include <nlohmann/json.hpp>

module UserInterface.Widget;

import std;
import :SlicedBrush;
import :WidgetResources;
import Serialization;

std::string SerializedKey(std::string_view prefix, std::string_view name)
{
	std::string key(name);
	if (prefix.empty())
		return key;

	key[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(key[0])));
	return std::string(prefix) + key;
}

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

// 브러시는 이름으로 저장한다. 이름을 모르는 핸들(저장), 이름이 없거나 모르는 이름(불러오기)이면 실패
bool SerializeBrushName(Serializer& serializer, IWidgetResources& resources, BrushHandle& brush, std::string_view key)
{
	std::string name;
	if (serializer.IsWrite())
	{
		const auto found = resources.BrushName(brush);
		if (!found)
			return false;
		name = *found;
	}

	serializer.Process(std::string(key), name);
	if (serializer.IsWrite())
		return true;

	const auto found = name.empty() ? std::nullopt : resources.FindBrush(name);
	if (!found)
		return false;

	brush = *found;
	return true;
}

void SerializeSliceMargins(Serializer& serializer, SliceMargins& margins, Core::Vector2& srcSize, std::string_view prefix)
{
	serializer.Process(SerializedKey(prefix, "sliceLeft"), margins.left);
	serializer.Process(SerializedKey(prefix, "sliceTop"), margins.top);
	serializer.Process(SerializedKey(prefix, "sliceRight"), margins.right);
	serializer.Process(SerializedKey(prefix, "sliceBottom"), margins.bottom);
	serializer.Process(SerializedKey(prefix, "srcWidth"), srcSize.x);
	serializer.Process(SerializedKey(prefix, "srcHeight"), srcSize.y);
}

bool SerializeSlicedBrush(Serializer& serializer, IWidgetResources& resources, SlicedBrush& brush, std::string_view prefix)
{
	BrushHandle handle = brush.brush;
	if (!SerializeBrushName(serializer, resources, handle, SerializedKey(prefix, "brush")))
		return false;

	bool sliced = brush.margins.has_value();
	SliceMargins margins = brush.margins.value_or(SliceMargins{});
	Core::Vector2 srcSize = brush.srcSize;

	serializer.Process(SerializedKey(prefix, "sliced"), sliced);
	SerializeSliceMargins(serializer, margins, srcSize, prefix);

	if (serializer.IsWrite())
		return true;

	brush = sliced ? SlicedBrush::Sliced(handle, margins, srcSize) : SlicedBrush::Plain(handle);
	return true;
}
