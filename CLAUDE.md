# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

Strawberry는 C++23 Modules 기반 DirectX 12 렌더링/UI 엔진이다. 문서와 주석은 한국어다. 자세한 설계는 `README.md`, 메모/할 일은 `Archives/` (Concept/Problem/Todo.txt)에 있다.

## 빌드 / 테스트

Visual Studio 2026(v18) + MSBuild, 솔루션은 `.slnx`, 플랫폼은 x64. 린터는 없다.

- **UI 빌드 + 테스트 한 번에**: `Test_UI.bat [gtest_filter]` (예: `Test_UI.bat UserInterfaceT_Toggle.*`). vswhere로 MSBuild를 찾고, Debug x64로 빌드한 뒤 `--gtest_brief=1`로 실행해 실패한 테스트만 출력한다. 콘솔에는 영어 오류만 나오고(`VSLANG=1033`, `-clp:ErrorsOnly`) 전체 빌드 로그는 `Testing\Game\x64\build_ui.log`에 저장된다. Bash에서는 배치 파일의 `%~dp0` 때문에 전체 경로로 실행하는 편이 안전하다.
- **런타임 컴파일 확인**: `Build_Strawberry.bat`은 루트의 `Strawberry.slnx`(테스트를 뺀 엔진/게임/툴 프로젝트 전체)를 빌드만 한다 (테스트 코드는 없다). 프로젝트를 추가하거나 위치를 옮기면 `Strawberry.slnx`도 함께 갱신한다. 약 80초로 오래 걸리므로 작업마다 돌리지 않고, 사용자가 "전체 컴파일 해 볼까?"처럼 요청할 때만 돌려서 이상 유무를 보고한다. 하루에 한 번 정도가 기준이다. 마지막 전체 컴파일 뒤 커밋이 3-4개 쌓이면 먼저 "전체 컴파일 해 볼까요?"라고 물어본다. 마지막으로 확인한 커밋은 `Archives/Todo.txt` 맨 아래 "전체 컴파일 마지막 확인: <커밋 해시>" 줄에 있고, 그 뒤 커밋 수는 `git rev-list --count <해시>..HEAD`로 센다. 전체 컴파일을 하고 이상이 없으면 이 줄의 해시를 현재 HEAD로 바꾼다 (이 줄은 항목이 아니라 기록이므로 윗부분을 지울 때도 남긴다). (입력/렌더/에셋/오디오 쪽을 고쳤다면 다음 전체 컴파일 때 확인이 필요하다고 알려 준다.) 로그는 `x64\build_strawberry.log`.
- 수동 빌드: `"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" Testing\Game\UserInterface-Test.slnx -p:Configuration=Debug -p:Platform=x64 -m -v:m -nologo` → 실행 파일은 `Testing\Game\x64\Debug\UserInterface-Test.exe`.
- 새 소스 파일은 `.vcxproj`와 `.vcxproj.filters`에 직접 등록해야 한다 (자동 포함되지 않는다). 빠뜨리면 컴파일 대상에서 조용히 빠지므로, 빌드 로그 파일(`build_ui.log`, `build_runtime.log`)에 새 파일이 나오는지 확인한다.

- `Strawberry.slnx` — 게임/엔진 본체 (Client, Contract, Core, DxInput, DxRender, FileAsset, SDLAudio, Serialization, Service, UserInterface, UIVisualizer)
- **테스트 솔루션은 자주 쓰는 것이 2개다.**
  - `Testing/Runtime-Test.slnx` — 백엔드 전체(DxInput, DxRender, FileAsset, SDLAudio, Serialization, Service 등)를 빌드하고 테스트한다 (`Testing/Runtime/*-Test`). UI 모듈은 포함하지 않는다. 입력/렌더/에셋/오디오 작업은 이 솔루션을 쓴다. 실행 파일은 `Testing\x64\Debug\<이름>-Test.exe`다 (예: `DxInput-Test.exe`).
  - `Testing/Game/UserInterface-Test.slnx` — UI 모듈만 빌드하고 테스트한다. UI 작업 시 이 솔루션만 빌드하면 된다 (Contract, UserInterface, Core, GoogleTest, UserInterface-Test)
- `Testing/Runtime/DxRender-Test.slnx` — Render 백엔드만 테스트하는 솔루션이다. 가끔 쓴다.
- `Testing/Runtime/FileAsset-Test.slnx` — 에셋 로딩/저장(FileAsset)만 테스트하는 솔루션이다 (Core, Contract, Serialization, Service 포함). 실행 파일은 `Testingd\Debug\FileAsset-Test.exe`다.
- `Tools/Tools.slnx` — 툴 솔루션이다 (`Tools/UIVisualizer`: 같은 UI 데이터를 Claude에게는 텍스트로, 사람에게는 화면으로 보여 주는 실행 파일). 실행 파일은 `Tool\x64\Debug\UIVisualizer.exe`다.
- `Testing/Runtime/Runtime.NoRender-Test.slnx` — 렌더를 뺀 런타임 솔루션이다. 당분간 쓰지 않으며, 현재 `Service`/`GoogleTest`에서 pch 오류(C1010)가 난다. 빌드/테스트에 사용하지 않는다.
- **`DxRender-Test.exe`와 `SDLAudio-Test.exe`는 실행하지 않는다.** 실행 시간이 길고 사람이 눈과 귀로 판단하도록 만든 테스트라서 자동 실행에 맞지 않는다. 빌드(컴파일 확인)는 해도 된다.
- 테스트는 GoogleTest(`Testing/GoogleTest`, 소스 포함) 실행 파일이다. 단일 테스트는 `UserInterface-Test.exe --gtest_filter=UserInterfaceT_Toggle.*` 처럼 실행한다.
- `Clean_x64_vs.bat` — 모든 `x64` 출력 폴더와 `.vs`를 삭제한다.
- 공통 컴파일 옵션/include 경로는 `Directory.Build.props`에 있다. include 루트는 저장소 루트, `Foundation`, `Game`, `Runtime`, `ThirdParty`다.

## 아키텍처

계층: 게임 클라이언트 → 엔진 서비스(Render/Input/Audio/Asset) → **Contract**(인터페이스·공유 데이터) ← 백엔드(DxRender, DxInput, SDLAudio). 서비스와 백엔드는 서로 직접 의존하지 않고 Contract만 공유한다. 모듈 의존 방향과 순환 참조가 핵심 관심사이며, `ImportGraph.py`로 점검한다. `ModuleGraph.txt`에 모듈 간 의존 관계가 적혀 있다. 커밋할 때마다 자동으로 갱신되고 수동으로 갱신하기도 한다. 새 `import`를 추가할 때는 이 파일을 참고해 방향을 깨지 않는지 확인한다.

**모듈 그래프 점검**: 한 덩어리의 작업을 끝내고 빌드/테스트가 통과하면, 매번이 아니라 이때 한 번 `ModuleGraph.txt`를 새로 뽑아 내용을 확인한다. 의존 방향이 계층(위 → 아래)을 거스르지 않는지, UI 모듈이 `DxRender`/`DxInput`에 의존하지 않는지, 마지막의 `cycles` 항목이 비었는지를 본다. 순환은 컴파일러도 잡지만 방향 위반은 이 파일로만 보인다. 실행은 저장소 루트에서 한다 (PowerShell에서 경로 두 개를 `;`로 이은 인자 하나로 묶어야 한다. Bash에서는 `;`가 명령 구분자라 잘못 실행된다):

```
& "C:\Users\nixie\AppData\Local\Programs\Python\Python314\python.exe" ImportGraph.py 'D:\Project\Development\Strawberry\Foundation;D:\Project\Development\Strawberry\Game;D:\Project\Development\Strawberry\Runtime'
```

디렉터리: `Foundation/` (Core, Serialization, Contract: Game과 Runtime이 함께 쓰는 기반으로 아무 데도 의존하지 않는다), `Game/` (App, Client, Service, UserInterface), `Runtime/` (DxInput, DxRender, FileAsset, SDLAudio), `Tools/` (UIVisualizer), `Testing/` (테스트 미러 구조). `Tools/`는 `Foundation`, `Game`, `Runtime`을 모두 쓰는 최상위 실행 파일 층이고 아무도 import하지 않으므로 `ImportGraph.py` 점검 대상에 넣지 않는다.

## 규약

- 소스는 UTF-8(`/utf-8`)이다. 한국어 주석/커밋 메시지를 쓴다.
- git의 `LF will be replaced by CRLF` 경고는 무시한다 (저장소는 LF, 작업 폴더는 CRLF/LF가 섞여 있지만 해롭지 않다). 줄바꿈을 통일하는 작업은 하지 않는다.
- 세부 규칙은 `.claude/rules/`에 있고 해당 경로의 파일을 다룰 때 적용된다: `import-order.md`(import 순서), `module-layout.md`(.ixx/.cpp 분리 기준), `testing.md`(테스트 코드 규칙), `ui-module.md`(UI 모듈), `ui-testing.md`(UI 테스트), `input-pipeline.md`(입력 파이프라인), `ui-visualizer.md`(UIVisualizer). 길어질 규칙은 CLAUDE.md에 늘리지 말고 이곳에 파일로 추가한다.

## 작업 방침

- **할 일 목록**: `Archives/Todo.txt`에는 계획이나 설명 없이 기능 단위 한 줄씩만 적는다. 구현이 끝난 항목은 `[x]`, 진행 중은 `[~]`, 예정은 `[ ]`, 보류(눈으로 확인할 수 있을 때 진행)는 `[-]`로 표시한다. 길어지면 사용자가 윗부분을 지운다. 작업 전에 읽는다. 목록에 없던 작업(파일 정리, 리팩토링 등)을 했을 때도 요약해서 한 줄 추가한다. 단 테스트 프로젝트(`Testing/`)에 한 작업은 전부 적지 않는다 ("테스트 언급 금지"가 여기에도 적용된다). 작업한 항목은 `[~]`로 두고, `[x]`로 바꾸는 것은 사용자가 커밋하면서 완료를 확인할 때 한다. 세부 계획은 문서로 남기지 않고 코드와 실제 동작을 보고 판단한다.
- **테스트 언급 금지**: 커밋 메시지에 테스트 관련 내용(테스트 추가, 테스트 통과 등)을 적지 않는다. 코드 주석에도 "이건 XXX 테스트에서 확인된다"처럼 테스트를 가리키는 주석을 쓰지 않는다.
- **커밋**: 사용자가 요청할 때만 커밋한다. 기능 하나(코드 + 프로젝트 등록 + 문서)는 한 커밋으로 묶고, `CLAUDE.md`나 설정처럼 작은 변경은 따로 커밋하지 않는다.
- **네이밍**: `Manager`, `System`처럼 의미가 담기지 않은 이름은 피하고, 역할이 드러나는 이름을 쓴다.
- **서브에이전트**: 파일 탐색이나 검색 같은 단순 작업에만 쓰고, 그때 모델을 haiku로 지정한다. 코드 리뷰, 설계 판단, 구현은 서브에이전트에 맡기지 않고 직접 한다 (기본 모델은 sonnet).
