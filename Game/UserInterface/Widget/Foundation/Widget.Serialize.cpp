module;

#include <nlohmann/json.hpp>

module UserInterface.Widget;

import std;
import :Widget;
import Serialization;
import Core.Utils;

// 저장 파일에 적히는 이름. 한 번 정하면 바꾸지 않는다 (옛 파일을 읽을 수 없게 된다)
template<>
constexpr std::array<const char*, Core::EnumSize<SizePolicy>> Core::EnumToStringMap<SizePolicy>()
{
    return Core::MakeEnumStringMap<SizePolicy>("Fixed", "Fit", "Fill");
}

// 멤버를 그대로 읽고 쓴다 (읽기/쓰기가 같은 코드). 읽을 때 키가 없으면 멤버가 그대로 남는다.
// enum은 이름으로 적히고, 모르는 이름은 Serializer가 오류로 알린다.
bool Widget::Serialize(Serializer& serializer, IWidgetResources&)
{
    serializer.Process("anchorX", m_anchor.anchor.x);
    serializer.Process("anchorY", m_anchor.anchor.y);
    serializer.Process("pivotX", m_anchor.pivot.x);
    serializer.Process("pivotY", m_anchor.pivot.y);
    serializer.Process("offsetX", m_anchor.offset.x);
    serializer.Process("offsetY", m_anchor.offset.y);
    serializer.Process("width", m_rect.width);
    serializer.Process("height", m_rect.height);
    serializer.Process("widthPolicy", m_widthPolicy);
    serializer.Process("heightPolicy", m_heightPolicy);
    serializer.Process("tooltip", m_tooltip);
    if (!serializer.IsWrite() || !m_name.empty())   // 이름 없는 위젯은 키를 적지 않는다
        serializer.Process("name", m_name);

    if (!IsValidName(m_name))
        return false;

    if (!serializer.IsWrite())
        Relayout();   // 직접 채운 값을 레이아웃에 반영
    return true;
}
