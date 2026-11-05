export module UserInterface.Widget:TextSerialize;

import std;
import :WidgetResources;
import Contract.Render.UI;
import Serialization;

// 글자를 그리는 위젯(Text, TextBox, Dropdown)이 함께 쓰는 저장/불러오기 도우미. 구현: TextSerialize.cpp
// 모두 실패하면 false: 폰트 이름을 모르거나(저장), 이름이 없거나 모르는 이름이거나(불러오기), enum 이름이 틀렸을 때.

export bool SerializeFontName(Serializer& serializer, IWidgetResources& resources, FontHandle& font, std::string_view key);
export bool SerializeTextRenderMode(Serializer& serializer, TextRenderMode& mode, std::string_view key);

// prefix가 비어 있지 않으면 키 앞에 붙는다 (SerializedKey 규칙)
export bool SerializeTextLayout(Serializer& serializer, TextLayout& layout, std::string_view prefix);
export bool SerializeTextStyle(Serializer& serializer, TextStyle& style, std::string_view prefix);   // 색과 효과 5개(외곽선, 그림자, 그라데이션, 글로우, 밑줄) 모두
