---
paths:
  - "Runtime/DxInput/**"
  - "Game/Contract/Input/**"
  - "Game/Service/Input/**"
  - "Testing/Runtime/DxInput-Test/**"
---

# 입력 파이프라인 (`Runtime/DxInput`, 모듈명 `DxInput`)

흐름: Win32 메시지 → `DxInput`(`IInputBackend::TakeFrame`) → `InputFrame` → `Service/Input`(`IInputService::Update`) → `MouseState`/`ButtonState`/`GetTextInput` → UI.

- **Contract** (`Game/Contract/Input`): `Shared`(`KeyCode`, `MouseButton`), `Backend`(`IInputBackend`, `InputFrame`, `CreateDxInputBackend`), `Service`(`IInputService`, `ButtonState`, `MouseState`). `InputFrame`은 순수 데이터로 `keyEvents`, `mouseButtonEvents`, `position`, `wheelDelta`(한 틱 = 120), `text`(UTF-8)만 담는다. 새 입력 종류를 추가하면 `InputFrame::Empty()`도 함께 고친다.
- **DxInput**: 정적 라이브러리. 의존은 `Contract.Input.Backend/Shared`, `Core.Utils`뿐이고 DirectXTK12(`Keyboard`, `Mouse`)를 래핑한다. 파일은 `InputBackend.ixx`(클래스 선언), `InputBackend.cpp`(구현), `KeyMap.ixx`(DXKey→`KeyCode`), `MouseButtonMap.ixx`(`MouseButton`→`DirectX::Mouse::State` 멤버 포인터)다. 새 키/버튼은 매핑 테이블에 한 줄 추가한다.
- **메시지 수집**: `SetWindowSubclass`로 윈도우를 서브클래싱해 메시지를 `Keyboard::ProcessMessage`/`Mouse::ProcessMessage`에 넘긴다. `WM_NCDESTROY`에서 스스로 해제한다.
- **문자 입력**: `WM_CHAR`를 UTF-8 문자열로 모아 `InputFrame::text`에 담는다. 제어 문자(0x20 미만, 0x7F)는 제외하고(Backspace/Enter/Tab/Esc는 `KeyCode`로 전달), 서로게이트 쌍은 합친다. 서비스는 `IInputService::GetTextInput()`으로 조회하며 다음 `Update`에서 교체된다. `WM_IME_*`(조합 중 문자열)는 아직 처리하지 않는다.
- **이벤트 생성**: 키보드와 마우스 버튼은 프레임마다 `GetState()`를 폴링해 이전 상태와 달라진 것만 이벤트로 만든다(키 자동 반복 없음). 휠은 직전 누적값과의 차이, 위치는 마지막 값을 유지한다.
- **구현체**: 클래스(`InputBackend`)는 모듈 밖으로 export하지 않고, `Contract.Input.Backend`가 선언한 팩토리 `CreateDxInputBackend(NativeWindowHandle)`로만 만든다.
- **서비스** (`Game/Service/Input`): `InputService`가 `InputFrame`을 `ButtonState`(pressed/released/held)로 변환한다. `MockInputBackend`, `StubInputBackend`는 이곳에 있다.
- **테스트**: `Testing/Runtime/DxInput-Test`(`Runtime-Test.slnx`에 속함). 서비스 단위 테스트(`InputServiceT`)가 `MockInputBackend`로 돌아간다.
