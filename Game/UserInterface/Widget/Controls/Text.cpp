module;

#include <nlohmann/json.hpp>

module UserInterface.Widget;

import std;
import :Text;
import :TextSerialize;
import Serialization;

// 글자 크기와 렌더 모드는 폰트와 별개로 적는다 (같은 폰트를 크기와 모드를 바꿔 그린다).
// 줄바꿈 폭(wrapWidth)은 생성 때의 폭으로 고정된 값이라 width와 따로 적는다. Fit이 width를 덮어써도 줄바꿈 위치가 달라지지 않게 하려는 것이다.
bool Text::Serialize(Serializer& serializer, IWidgetResources& resources)
{
    if (!Widget::Serialize(serializer, resources))
        return false;

    serializer.Process("text", m_text);
    serializer.Process("size", m_size);
    serializer.Process("wrapWidth", m_wrapWidth);

    if (!SerializeFontName(serializer, resources, m_font, "font")
        || !SerializeTextRenderMode(serializer, m_mode, "renderMode")
        || !SerializeTextLayout(serializer, m_layout, "")
        || !SerializeTextStyle(serializer, m_style, ""))
        return false;

    if (!serializer.IsWrite())
    {
        m_measured.reset();   // 읽은 폰트/크기/줄바꿈 폭으로 다시 잰다
        InvalidateLayout();
    }
    return true;
}
