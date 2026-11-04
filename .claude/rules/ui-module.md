---
paths:
  - "Game/UserInterface/**"
---

# UI 모듈 (`Game/UserInterface`, 모듈명 `Game.UserInterface`)

- 의존 가능: `Contract.Input.Service`, `Contract.Render.UI`, `Core.Math`. **`DxRender`/`DxInput`에 의존하지 않는다.** 그리기는 `IOverlayView`, 텍스트 측정은 `ITextMeasurer`, 입력은 `MouseState`(순수 데이터)로만 접근한다. 폰트와 스킨은 클라이언트가 주입한다.
- `UserInterface`가 레이어(Hud < Window < Popup < Tooltip)별 위젯 트리를 소유한다. 위쪽 레이어가 입력을 먼저 받는다. 보조 객체: `InputRouter`(hover/캡처/클릭/휠), `FocusRouter`(Tab/키 버블링/문자 입력/모달), `TooltipController`.
- 수명: 부모가 자식을 `unique_ptr`로 소유하고 `Detach`는 소유권을 돌려준다. 라우터는 `weak_ptr` 핸들만 보관한다. **콜백 중에 위젯이 Detach/파괴돼도 안전해야 한다** (전용 테스트가 있다).
- 레이아웃: 위치는 anchor/pivot/offset, 크기는 축별 `SizePolicy`(Fixed/Fit/Fill). 변경 시 `MeasureTree`(아래→위) 후 `ArrangeTree`(위→아래). 컨테이너는 `ArrangeChildren`을 재정의한다. 좌표는 정수 픽셀로 맞춘다.
- 입력 의미: 누름은 위젯에서 부모로 버블링되고 소비한 위젯이 캡처한다. 클릭은 뗀 위치가 그 위젯 위일 때만 성립한다. `Backdrop`은 모달이며 포커스와 키를 가둔다.
- 키와 문자: 키(`OnKeyDown`)는 포커스 위젯에서 부모로 버블되지만, 문자(`OnTextInput`, UTF-8)는 포커스 위젯만 받고 버블되지 않는다. 포커스가 없거나 모달 밖이면 버린다.
- 위젯 파일은 `Widget/{Containers,Controls,Drawing,Foundation}`에 있고 `UserInterface.Widget.ixx`가 묶는다. 새 위젯은 이 구조와 `.vcxproj`(+`.filters`) 등록을 따른다.
- 시간: `UserInterface::Tick(dt)`는 툴팁과 포커스 위젯(`OnTick`)에게 시간을 전달한다. 캐럿 깜빡임처럼 포커스 위젯에만 필요한 갱신이 이 경로를 쓴다.
- **`SetXxx()` 규약**: 프로그램에서 호출하는 `SetXxx()`는 콜백을 발생시키지 않고, 사용자 입력으로 값이 바뀔 때만 `OnChanged` 류가 발생한다 (Toggle, Slider, TextBox 참고).
