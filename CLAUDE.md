# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

Strawberry는 C++23 Modules 기반 DirectX 12 렌더링/UI 엔진이다. 문서와 주석은 한국어다. 자세한 설계는 `README.md`, 메모/할 일은 `Archives/` (Concept/Problem/Todo.txt)에 있다.

## 빌드 / 테스트

Visual Studio 2026(v18) + MSBuild, 솔루션은 `.slnx`, 플랫폼은 x64. 린터는 없다.

- **UI 빌드 + 테스트 한 번에**: `Test_UI.bat [gtest_filter]` (예: `Test_UI.bat UserInterfaceT_Toggle.*`). vswhere로 MSBuild를 찾고, Debug x64로 빌드한 뒤 `--gtest_brief=1`로 실행해 실패한 테스트만 출력한다.
- 수동 빌드: `"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" Testing\Game\UserInterface-Test.slnx -p:Configuration=Debug -p:Platform=x64 -m -v:m -nologo` → 실행 파일은 `Testing\Game\x64\Debug\UserInterface-Test.exe`.
- 새 소스 파일은 `.vcxproj`와 `.vcxproj.filters`에 직접 등록해야 한다 (자동 포함되지 않는다). 빠뜨리면 컴파일 대상에서 조용히 빠지므로, 빌드 로그에 새 파일이 나오는지 확인한다.

- `Strawberry.slnx` — 게임/엔진 본체 (Client, Contract, Core, DxInput, DxRender, FileAsset, SDLAudio, Serialization)
- **테스트 솔루션은 자주 쓰는 것이 2개다.**
  - `Testing/Runtime-Test.slnx` — 백엔드 전체(DxInput, DxRender, FileAsset, SDLAudio, Serialization, Service 등)를 빌드하고 테스트한다 (`Testing/Runtime/*-Test`). UI 모듈은 포함하지 않는다. 입력/렌더/에셋/오디오 작업은 이 솔루션을 쓴다. 실행 파일은 `Testing\x64\Debug\<이름>-Test.exe`다 (예: `DxInput-Test.exe`).
  - `Testing/Game/UserInterface-Test.slnx` — UI 모듈만 빌드하고 테스트한다. UI 작업 시 이 솔루션만 빌드하면 된다 (Contract, UserInterface, Core, GoogleTest, UserInterface-Test)
- `Testing/Runtime/DxRender-Test.slnx` — Render 백엔드만 테스트하는 솔루션이다. 가끔 쓴다.
- `Testing/Runtime/Runtime.NoRender-Test.slnx` — 렌더를 뺀 런타임 솔루션이다. 당분간 쓰지 않으며, 현재 `Service`/`GoogleTest`에서 pch 오류(C1010)가 난다. 빌드/테스트에 사용하지 않는다.
- 테스트는 GoogleTest(`Testing/GoogleTest`, 소스 포함) 실행 파일이다. 단일 테스트는 `UserInterface-Test.exe --gtest_filter=UserInterfaceT_Toggle.*` 처럼 실행한다.
- `Clean_x64_vs.bat` — 모든 `x64` 출력 폴더와 `.vs`를 삭제한다.
- 공통 컴파일 옵션/include 경로는 `Directory.Build.props`에 있다. include 루트는 저장소 루트, `Game`, `Runtime`, `ThirdParty`다.

## 아키텍처

계층: 게임 클라이언트 → 엔진 서비스(Render/Input/Audio/Asset) → **Contract**(인터페이스·공유 데이터) ← 백엔드(DxRender, DxInput, SDLAudio). 서비스와 백엔드는 서로 직접 의존하지 않고 Contract만 공유한다. 모듈 의존 방향과 순환 참조가 핵심 관심사이며, `ImportGraph.py`로 점검한다. `ModuleGraph.txt`에 모듈 간 의존 관계가 적혀 있다. 커밋할 때마다 자동으로 갱신되고 수동으로 갱신하기도 한다. 새 `import`를 추가할 때는 이 파일을 참고해 방향을 깨지 않는지 확인한다.

디렉터리: `Game/` (App, Client, Contract, Service, UserInterface), `Runtime/` (Core, DxInput, DxRender, FileAsset, SDLAudio, Serialization), `Testing/` (테스트 미러 구조).

## UI 모듈 (`Game/UserInterface`, 모듈명 `Game.UserInterface`)

- 의존 가능: `Contract.Input.Service`, `Contract.Render.UI`, `Core.Math`. **`DxRender`/`DxInput`에 의존하지 않는다.** 그리기는 `IOverlayView`, 텍스트 측정은 `ITextMeasurer`, 입력은 `MouseState`(순수 데이터)로만 접근한다. 폰트와 스킨은 클라이언트가 주입한다.
- `UserInterface`가 레이어(Hud < Window < Popup < Tooltip)별 위젯 트리를 소유한다. 위쪽 레이어가 입력을 먼저 받는다. 보조 객체: `InputRouter`(hover/캡처/클릭/휠), `FocusRouter`(Tab/키 버블링/모달), `TooltipController`.
- 수명: 부모가 자식을 `unique_ptr`로 소유하고 `Detach`는 소유권을 돌려준다. 라우터는 `weak_ptr` 핸들만 보관한다. **콜백 중에 위젯이 Detach/파괴돼도 안전해야 한다** (전용 테스트가 있다).
- 레이아웃: 위치는 anchor/pivot/offset, 크기는 축별 `SizePolicy`(Fixed/Fit/Fill). 변경 시 `MeasureTree`(아래→위) 후 `ArrangeTree`(위→아래). 컨테이너는 `ArrangeChildren`을 재정의한다. 좌표는 정수 픽셀로 맞춘다.
- 입력 의미: 누름은 위젯에서 부모로 버블링되고 소비한 위젯이 캡처한다. 클릭은 뗀 위치가 그 위젯 위일 때만 성립한다. `Backdrop`은 모달이며 포커스와 키를 가둔다.
- 위젯 파일은 `Widget/{Containers,Controls,Drawing,Foundation}`에 있고 `UserInterface.Widget.ixx`가 묶는다. 새 위젯은 이 구조와 `.vcxproj`(+`.filters`) 등록을 따른다.

## UI 테스트 (`Testing/Game/UserInterface-Test`)

- 파일: `UserInterfaceT.<기능>.cpp`(통합 성격), `Widget/<위젯>T.cpp`(위젯 단위). 이름은 `Fixture, 동작_조건_결과` 형식이다 (예: `Click_FlipsCheckedEachTime`).
- 공용 픽스처는 `UserInterfaceT.ixx`(`UserInterfaceT_Input` 등, `Click(...)` 헬퍼), 지원 코드는 `UserInterfaceT.Support.ixx`/`Widget/WidgetT.Support.ixx`, 테스트 더블은 `Doubles/`에 있다.

## 입력 파이프라인 (`Runtime/DxInput`, 모듈명 `DxInput`)

흐름: Win32 메시지 → `DxInput`(`IInputBackend::TakeFrame`) → `InputFrame` → `Service/Input`(`IInputService::Update`) → `MouseState`/`ButtonState` → UI.

- **Contract** (`Game/Contract/Input`): `Shared`(`KeyCode`, `MouseButton`), `Backend`(`IInputBackend`, `InputFrame`, `CreateDxInputBackend`), `Service`(`IInputService`, `ButtonState`, `MouseState`). `InputFrame`은 순수 데이터로 `keyEvents`, `mouseButtonEvents`, `position`, `wheelDelta`(한 틱 = 120)만 담는다. 새 입력 종류를 추가하면 `InputFrame::Empty()`도 함께 고친다.
- **DxInput**: 정적 라이브러리. 의존은 `Contract.Input.Backend/Shared`, `Core.Utils`뿐이고 DirectXTK12(`Keyboard`, `Mouse`)를 래핑한다. 파일은 `InputBackend.ixx`(클래스 선언), `InputBackend.cpp`(구현), `KeyMap.ixx`(DXKey→`KeyCode`), `MouseButtonMap.ixx`(`MouseButton`→`DirectX::Mouse::State` 멤버 포인터)다. 새 키/버튼은 매핑 테이블에 한 줄 추가한다.
- **메시지 수집**: `SetWindowSubclass`로 윈도우를 서브클래싱해 메시지를 `Keyboard::ProcessMessage`/`Mouse::ProcessMessage`에 넘긴다. `WM_NCDESTROY`에서 스스로 해제한다. 현재 `WM_CHAR`/`WM_IME_*`는 처리하지 않는다.
- **이벤트 생성**: 키보드와 마우스 버튼은 프레임마다 `GetState()`를 폴링해 이전 상태와 달라진 것만 이벤트로 만든다(키 자동 반복 없음). 휠은 직전 누적값과의 차이, 위치는 마지막 값을 유지한다.
- **구현체**: 클래스(`InputBackend`)는 모듈 밖으로 export하지 않고, `Contract.Input.Backend`가 선언한 팩토리 `CreateDxInputBackend(NativeWindowHandle)`로만 만든다.
- **서비스** (`Game/Service/Input`): `InputService`가 `InputFrame`을 `ButtonState`(pressed/released/held)로 변환한다. `MockInputBackend`, `StubInputBackend`는 이곳에 있다.
- **테스트**: `Testing/Runtime/DxInput-Test`(`Runtime-Test.slnx`에 속함). 서비스 단위 테스트(`InputServiceT`)가 `MockInputBackend`로 돌아간다.

## 규약 (기존 코드에서 확인된 것)

- 프로그램에서 호출하는 `SetXxx()`는 콜백을 발생시키지 않고, 사용자 입력으로 값이 바뀔 때만 `OnChanged` 류가 발생한다 (Toggle, Slider 테스트 참고).
- 소스는 UTF-8(`/utf-8`)이다. 한국어 주석/커밋 메시지를 쓴다.

## 작업 방침

- **테스트 코드**: 중복 없이 작성한다. 기존 테스트와 겹치면 공통 부분을 리팩토링하거나 중복된 테스트를 삭제한다.
- **테스트를 위한 접근 확장 금지**: 테스트를 위해 함수를 `public`으로 만들거나 공개(export)하지 않는다.
- **테스트 언급 금지**: 커밋 메시지에 테스트 관련 내용(테스트 추가, 테스트 통과 등)을 적지 않는다. 코드 주석에도 "이건 XXX 테스트에서 확인된다"처럼 테스트를 가리키는 주석을 쓰지 않는다.
- **커밋**: 사용자가 요청할 때만 커밋한다. 기능 하나(코드 + 프로젝트 등록 + 문서)는 한 커밋으로 묶고, `CLAUDE.md`나 설정처럼 작은 변경은 따로 커밋하지 않는다.
- **네이밍**: `Manager`, `System`처럼 의미가 담기지 않은 이름은 피하고, 역할이 드러나는 이름을 쓴다.
