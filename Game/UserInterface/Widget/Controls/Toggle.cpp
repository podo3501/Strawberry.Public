module;

#include <nlohmann/json.hpp>

module UserInterface.Widget;

import std;
import :Toggle;
import Serialization;

// 체크 상태를 적는다. 읽을 때는 SetChecked와 같이 콜백을 부르지 않는다. 변경 콜백(SetOnChanged)은 저장하지 않는다
bool Toggle::Serialize(Serializer& serializer, IWidgetResources& resources)
{
	if (!Widget::Serialize(serializer, resources))
		return false;

	serializer.Process("checked", m_checked);
	return SerializeButtonBrushes(serializer, resources, m_skin.off, "off")
		&& SerializeButtonBrushes(serializer, resources, m_skin.on, "on");
}
