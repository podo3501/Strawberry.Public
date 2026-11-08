export module UIVisualizer.WidgetPathFinder;

import std;
import UserInterface;

// SelectionReport::WidgetPath가 만든 경로로 위젯을 다시 찾는다 (구현: WidgetPathFinder.cpp)
// 레이아웃을 다시 읽어 위젯이 새로 만들어진 뒤에도 같은 위젯을 가리키려는 것이다.
// layerRoot 기준 경로이고, 조각은 '/'로 잇는다. 조각은 이름이거나, 이름이 없는 위젯의 "종류#순서"(순서는 부모의 children 안 위치, 0부터)다.
// 이름 조각은 그 이름의 자식을, "종류#순서" 조각은 그 자리의 자식을 고른다 (이름이 있는 자식이나 종류가 다른 자식은 고르지 않는다).
// 빈 경로, 빈 조각, 찾지 못한 조각이 있으면 nullptr
export const Widget* FindWidgetByPath(const Widget& layerRoot, std::string_view path);

// 값을 바꿀 위젯이 필요할 때의 같은 함수 (layerRoot가 바꿀 수 있는 트리일 때만 쓸 수 있다)
export Widget* FindWidgetByPath(Widget& layerRoot, std::string_view path);
