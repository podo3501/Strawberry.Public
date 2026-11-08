export module UIVisualizer.SelectionReport;

import std;
import UserInterface;

// 고른 위젯 목록을 Claude가 읽는 텍스트로 만든다 (구현: SelectionReport.cpp)
// 사람은 화면의 번호로 말하고, Claude는 이 텍스트에서 번호를 JSON 안의 위젯으로 바꾼다.
//
// 모양 (위젯 하나가 한 줄, 탭으로 구분):
//   # UIVisualizer 선택 2026-11-08 14:03:21
//   layout: Testing/Resource/UI/Sample/Window.json
//   1	Button	Button	screen=40,40,256x70	offset=40,40
//   2	Panel/Image#2	Image	screen=300,120,64x64	offset=20,10
// 열: 번호(고른 순서, 1부터), 경로, 종류, 화면 위치와 크기, 앵커 기준 오프셋(JSON의 offsetX/offsetY와 같은 값)

// layerRoot 아래에서 widget까지 이름을 '/'로 이은 경로. layerRoot 기준이고 layerRoot 자신은 적지 않는다.
// 이름이 없는 위젯은 "종류#순서"로 적고, 순서는 부모의 children 배열 안 위치(0부터)라서 JSON에서 그대로 찾을 수 있다.
// layerRoot 아래에 widget이 없으면 빈 문자열
export std::string WidgetPath(const Widget& layerRoot, const Widget& widget);

// items는 고른 순서대로(번호는 순서 + 1). 비어 있으면 "(선택 없음)"만 적어서 지난 선택이 남지 않게 한다.
// time은 파일이 지금 것인지 알아보는 표시이고, 만드는 쪽이 정해서 넘긴다
export std::string FormatSelection(const Widget& layerRoot, std::span<const Widget* const> items,
    std::string_view layoutFile, std::string_view time);
