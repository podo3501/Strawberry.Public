# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

Strawberry는 C++23 Modules 기반 DirectX 12 렌더링/UI 엔진이다. 문서와 주석은 한국어다. 자세한 설계는 `README.md`, 메모/할 일은 `Archives/` (Concept/Problem/Todo.txt)에 있다.

## 빌드 / 테스트

Visual Studio 2026(v18) + MSBuild, 솔루션은 `.slnx`, 플랫폼은 x64. 린터는 없다.

- **UI 빌드 + 테스트 한 번에**: `Test_UI.bat [gtest_filter]` (예: `Test_UI.bat UserInterfaceT_Toggle.*`). vswhere로 MSBuild를 찾고, Debug x64로 빌드한 뒤 `--gtest_brief=1`로 실행해 실패한 테스트만 출력한다. 콘솔에는 영어 오류만 나오고(`VSLANG=1033`, `-clp:ErrorsOnly`) 전체 빌드 로그는 `Testing\Game\x64\build_ui.log`에 저장된다. Bash에서는 배치 파일의 `%~dp0` 때문에 전체 경로로 실행하는 편이 안전하다.
- **오디오 빌드 + 테스트 한 번에**: `Test_Audio.bat [gtest_filter]` (예: `Test_Audio.bat VoicePoolAgent.*`). `Runtime-Test.slnx`를 빌드하고 기본 필터 `*Agent.*`로 `SDLAudio-Test.exe`를 실행한다 (소리 장치 없이 `SDL_AUDIODRIVER=dummy`로 돈다). 사람이 듣는 `AudioServiceUser`는 필터에서 항상 빠진다. 콘솔에는 오류와 실패한 테스트만 나오고 전체 빌드 로그는 `Testing\x64\build_audio.log`에 저장된다.
- **런타임 컴파일 확인**: `Build_Strawberry.bat`은 루트의 `Strawberry.slnx`(테스트를 뺀 엔진/게임/툴 프로젝트 전체)를 빌드만 한다 (테스트 코드는 없다). 프로젝트를 추가하거나 위치를 옮기면 `Strawberry.slnx`도 함께 갱신한다. 약 80초로 오래 걸리므로 작업마다 돌리지 않고, 사용자가 "전체 컴파일 해 볼까?"처럼 요청할 때만 돌려서 이상 유무를 보고한다. 마지막으로 확인한 커밋은 `Archives/Todo.txt` 맨 아래 "전체 컴파일 마지막 확인: <커밋 해시>" 줄에 있다. 전체 컴파일을 하고 이상이 없으면 이 줄의 해시를 현재 HEAD로 바꾼다 (이 줄은 항목이 아니라 기록이므로 윗부분을 지울 때도 남긴다). (입력/렌더/에셋/오디오 쪽을 고쳤다면 다음 전체 컴파일 때 확인이 필요하다고 알려 준다.) 로그는 `x64\build_strawberry.log`.
- 새 소스 파일은 `.vcxproj`와 `.vcxproj.filters`에 직접 등록해야 한다 (자동 포함되지 않는다). 빠뜨리면 컴파일 대상에서 조용히 빠지므로, 빌드 로그 파일(`build_ui.log`, `build_runtime.log`)에 새 파일이 나오는지 확인한다.

- **다른 솔루션을 쓸 때**: 솔루션 목록(`Strawberry.slnx`, `Runtime-Test.slnx`, `FileAsset-Test.slnx` 등), 수동 빌드, 단일 테스트 실행, 공통 컴파일 옵션은 `.claude/rules/solutions.md`에 있다. 입력/렌더/에셋/오디오 작업은 `Testing/Runtime-Test.slnx`를 쓴다. 그 파일을 먼저 읽는다.
- **`DxRender-Test.exe`는 실행하지 않는다.** 실행 시간이 길고 사람이 눈으로 판단하도록 만든 테스트라서 자동 실행에 맞지 않는다. 빌드(컴파일 확인)는 해도 된다.
- **`SDLAudio-Test.exe`는 직접 실행하지 않고 `Test_Audio.bat`만 쓴다.** 필터 없이 실행하면 사람이 듣는 `AudioServiceUser`까지 돌아서 실제로 소리가 난다.

## 아키텍처

계층: 게임 클라이언트 → 엔진 서비스(Render/Input/Audio/Asset) → **Contract**(인터페이스·공유 데이터) ← 백엔드(DxRender, DxInput, SDLAudio). 서비스와 백엔드는 서로 직접 의존하지 않고 Contract만 공유한다. 모듈 의존 방향과 순환 참조가 핵심 관심사이며, `ImportGraph.py`로 점검한다. `ModuleGraph.txt`에 모듈 간 의존 관계가 적혀 있다. 커밋할 때마다 자동으로 갱신되고 수동으로 갱신하기도 한다. 새 `import`를 추가할 때는 이 파일을 참고해 방향을 깨지 않는지 확인한다.

**모듈 그래프 점검**: 한 덩어리의 작업을 끝내고 빌드/테스트가 통과하면, 매번이 아니라 이때 한 번 `ModuleGraph.txt`를 새로 뽑아 내용을 확인한다. 의존 방향이 계층(위 → 아래)을 거스르지 않는지, UI 모듈이 `DxRender`/`DxInput`에 의존하지 않는지, 마지막의 `cycles` 항목이 비었는지를 본다. 순환은 컴파일러도 잡지만 방향 위반은 이 파일로만 보인다. 실행은 저장소 루트에서 한다 (PowerShell에서 경로 두 개를 `;`로 이은 인자 하나로 묶어야 한다. Bash에서는 `;`가 명령 구분자라 잘못 실행된다):

```
& "C:\Users\nixie\AppData\Local\Programs\Python\Python314\python.exe" ImportGraph.py 'D:\Project\Development\Strawberry\Foundation;D:\Project\Development\Strawberry\Game;D:\Project\Development\Strawberry\Runtime'
```

디렉터리: `Foundation/` (Core, Serialization, Contract: Game과 Runtime이 함께 쓰는 기반으로 아무 데도 의존하지 않는다), `Game/` (App, Client, Service, UserInterface, Visualizer/UIVisualizer), `Runtime/` (DxInput, DxRender, FileAsset, SDLAudio), `Testing/` (테스트 미러 구조).

## 규약

- 소스는 UTF-8(`/utf-8`)이다. 한국어 주석/커밋 메시지를 쓴다.
- git의 `LF will be replaced by CRLF` 경고는 무시한다 (저장소는 LF, 작업 폴더는 CRLF/LF가 섞여 있지만 해롭지 않다). 줄바꿈을 통일하는 작업은 하지 않는다.
- 세부 규칙은 `.claude/rules/`에 있고 해당 경로의 파일을 다룰 때 적용된다: `import-order.md`(import 순서), `module-layout.md`(.ixx/.cpp 분리 기준), `path-usage.md`(`filesystem::path` 사용 범위와 `AssetPath`), `solutions.md`(솔루션 목록과 빌드/실행), `testing.md`(테스트 코드 규칙), `ui-module.md`(UI 모듈), `ui-testing.md`(UI 테스트), `input-pipeline.md`(입력 파이프라인), `ui-visualizer.md`(UIVisualizer). 길어질 규칙은 CLAUDE.md에 늘리지 말고 이곳에 파일로 추가한다.

## 작업 방침

- **할 일 목록**: `Archives/Todo.txt`에는 계획이나 설명 없이 기능 단위 한 줄씩만 적는다. 구현이 끝난 항목은 `[x]`, 진행 중은 `[~]`, 예정은 `[ ]`, 보류(눈으로 확인할 수 있을 때 진행)는 `[-]`로 표시한다. 길어지면 사용자가 윗부분을 지운다. 작업 전에 읽는다. 목록에 없던 작업(파일 정리, 리팩토링 등)을 했을 때도 요약해서 한 줄 추가한다. 단 테스트 프로젝트(`Testing/`)에 한 작업은 전부 적지 않는다 ("테스트 언급 금지"가 여기에도 적용된다). 작업한 항목은 `[~]`로 두고, `[x]`로 바꾸는 것은 사용자가 커밋하면서 완료를 확인할 때 한다. 세부 계획은 문서로 남기지 않고 코드와 실제 동작을 보고 판단한다. 백엔드(`Runtime/`)의 테스트 자동화 작업은 같은 형식으로 `Archives/Backend-Todo.txt`에 정리한다.
- **실패 처리**: 실패할 수 있는 함수는 `bool`(또는 결과 타입)로 알리고, 호출하는 쪽은 결과를 버리지 않고 위로 전달한다. 실패하면 원인을 `Core::Logger::Error`로 남기는데, 이 로그는 Claude가 읽는 용도이므로 `bool`과 로그를 둘 다 한다. 시작할 때의 초기화는 하나라도 실패하면 `false`로 엄격하게 처리해 최상위(`App`의 `Initialize`)가 시작하지 않는다. 실행 중 되돌릴 수 있는 실패(UI JSON 다시 읽기)는 이전 상태를 유지하고 화면에 안내한다. 실패해도 계속하도록 `void`로 두고 결과를 조용히 버리지 않는다.
- **테스트 언급 금지**: 커밋 메시지에 테스트 관련 내용(테스트 추가, 테스트 통과 등)을 적지 않는다. 코드 주석에도 "이건 XXX 테스트에서 확인된다"처럼 테스트를 가리키는 주석을 쓰지 않는다.
- **커밋**: 사용자가 요청할 때만 커밋한다. 기능 하나(코드 + 프로젝트 등록 + 문서)는 한 커밋으로 묶고, `CLAUDE.md`나 설정처럼 작은 변경은 따로 커밋하지 않는다. 묶는 기준은 "하나의 목적으로 설명되는 변경"이다: 같은 목적의 코드, 테스트, 문서, 샘플 데이터는 한 커밋에 넣고, 따로 되돌릴 만한 변경(렌더 엔진과 UI 위젯 등)만 나눈다. 색이나 수치처럼 눈으로 보며 계속 고치는 조정은 끝날 때까지 모아 하나로 묶는다. 메시지를 한 문장으로 쓸 수 없으면(와/과가 세 번 이상) 나눌 신호이고, 반대로 잘게 쪼개지도 않는다(주제 단위 2~3개 정도). 묶음이 애매하면 커밋하기 전에 몇 개로 나눌지 한 줄로 먼저 알린다. 푸시한 커밋은 다시 나누지 않는다. 커밋 메시지는 무엇을 했는지 한눈에 보이는 한 줄 요약만 쓰고 본문은 쓰지 않는다. 끝의 공동 작성자 줄은 모델 이름과 버전 없이 `Co-Authored-By: Claude Code <noreply@anthropic.com>` 한 줄만 붙인다.
- **네이밍**: `Manager`, `System`처럼 의미가 담기지 않은 이름은 피하고, 역할이 드러나는 이름을 쓴다.
- **멤버는 값이 기본**: `std::unique_ptr` 멤버는 팩토리가 `unique_ptr`로 돌려주는 객체(`IAssetStorage`, `IRenderService`, 백엔드 같은 인터페이스)이거나 다형성 때문에 힙이 꼭 필요할 때만 쓴다. 그 밖의 멤버는 값으로 둔다. 값으로 두려고 `std::optional`로 생성을 미루지 않는다 (`unique_ptr`처럼 "만들어지지 않은 상태"만 생긴다). 의존하는 객체를 생성자에서 먼저 만들어 값 멤버를 초기화 리스트로 만드는 쪽으로 설계하고(멤버 선언 순서가 곧 생성 순서이자 해제의 역순이다), 정말 어렵다면 `unique_ptr`을 쓴다.
- **주석은 짧게**: 클래스 위의 주석은 길어도 4줄을 넘기지 않고, 함수(선언) 위나 옆의 주석은 길어도 2줄을 넘기지 않는다. 자세한 설명은 `.claude/rules/`의 문서로 옮기고 주석에는 핵심만 적는다.
- **커지면 나눈다**: 한 클래스의 멤버 변수가 10개 이상(참조 멤버도 센다)이거나 `.cpp`가 300줄 이상이면 역할별로 묶어 다른 클래스나 파일로 뺄 수 있는지 먼저 고민한다. 나누면 오히려 어색한 경우(하나의 상태를 함께 바꾸는 값들, 길어도 한 가지 일만 하는 파일 등)는 나누지 않아도 되고, 그때는 이유를 주석으로 남긴다.
- **플랜 모드 답변**: 계획은 "정해야 할 것"과 "알아야 할 것" 두 묶음으로 짧게 대화에 쓰고, 질문은 말로 묻는다. `AskUserQuestion`(1, 2, 3 선택 창)과 `ExitPlanMode`는 쓰지 않는다. 사용자가 말로 답하거나 플랜 모드를 나가면 바로 구현한다. 계획 파일은 만들지 않는다.
- **말투**: 대화 답변은 딱딱한 개조식이나 반말이 아니라 부드러운 경어체(~합니다, ~해요)로 쓴다. 호칭은 붙이지 않고 주어도 빼는 편이다. 커밋 메시지와 코드 주석에는 적용하지 않는다.
- **서브에이전트**: 파일 탐색이나 검색 같은 단순 작업에만 쓰고, 그때 모델을 haiku로 지정한다. 코드 리뷰, 설계 판단, 구현은 서브에이전트에 맡기지 않고 직접 한다 (기본 모델은 sonnet).
