module;

#include <nlohmann/json.hpp>

module UserInterface.Widget;

import std;
import :Button;
import Serialization;

// 클릭 콜백(SetOnClick)은 저장하지 않는다. 불러온 뒤 코드에서 다시 연결한다
bool Button::Serialize(Serializer& serializer, IWidgetResources& resources)
{
	if (!Widget::Serialize(serializer, resources))
		return false;

	return SerializeButtonBrushes(serializer, resources, m_brushes, "");
}
