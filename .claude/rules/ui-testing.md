---
paths:
  - "Testing/Game/**"
---

# UI 테스트 (`Testing/Game/UserInterface-Test`)

- **나누는 기준은 `UserInterface`를 쓰는가이다.**
  - `Widget/<위젯>T.cpp`: 위젯을 직접 만들어 검사한다. `UserInterface`를 쓰지 않는다.
  - `UserInterface/UserInterfaceT.<기능>.cpp`: `UserInterface`(`m_ui`)에 위젯을 붙여서 입력/포커스/레이아웃 흐름까지 거쳐 검사한다. Toggle, Slider, TextBox처럼 위젯 하나를 다루더라도 입력을 거치면 여기에 둔다.
- `UserInterface/` 안의 폴더는 위젯 폴더(`Widget/{Containers,Controls,...}`)를 따른다. 위젯 하나를 다루는 테스트는 해당 하위 폴더(`Containers/`, `Controls/`)에, 분류가 안 되는 것(Input, Focus, Tooltip, Layout, Tree 등)은 `UserInterface/` 루트에 둔다. 새 파일은 `.vcxproj`와 `.filters`(필터 폴더 포함)에 등록한다.
- 공용 픽스처(`UserInterfaceT.ixx`, `UserInterfaceT.Support.ixx`)는 테스트 프로젝트 루트에 있다.
- 이름은 `Fixture, 동작_조건_결과` 형식이다 (예: `Click_FlipsCheckedEachTime`).
- 공용 픽스처는 `UserInterfaceT.ixx`(`UserInterfaceT_Input` 등, `Click(...)` 헬퍼), 지원 코드는 `UserInterfaceT.Support.ixx`/`Widget/WidgetT.Support.ixx`, 테스트 더블은 `Doubles/`에 있다.
- 빌드와 실행은 `Test_UI.bat [gtest_filter]`로 한다.
