---
paths:
  - "Testing/**"
  - "**/*.slnx"
  - "**/*.bat"
  - "**/*.vcxproj"
  - "Directory.Build.props"
---

# 솔루션 목록

- `Strawberry.slnx` — 게임/엔진 본체 (Client, Contract, Core, DxInput, DxRender, FileAsset, SDLAudio, Serialization, Service, UserInterface, UIVisualizer, App)
- **테스트 솔루션은 자주 쓰는 것이 2개다.**
  - `Testing/Runtime-Test.slnx` — 백엔드 전체(DxInput, DxRender, FileAsset, SDLAudio, Serialization, Service 등)를 빌드하고 테스트한다 (`Testing/Runtime/*-Test`). UI 모듈은 포함하지 않는다. 입력/렌더/에셋/오디오 작업은 이 솔루션을 쓴다. 실행 파일은 `Testing\x64\Debug\<이름>-Test.exe`다 (예: `DxInput-Test.exe`).
  - `Testing/Game/UserInterface-Test.slnx` — UI 모듈만 빌드하고 테스트한다. UI 작업 시 이 솔루션만 빌드하면 된다 (Contract, UserInterface, Core, GoogleTest, UserInterface-Test)
  - `Testing/Game/Visualizer/UIVisualizer-Test.slnx` — 게임 안에서 UI JSON을 보여 주는 UIVisualizer 모듈(`Game/Visualizer/UIVisualizer`)만 빌드하고 테스트한다. 창 없이 돈다. `Test_UIVisualizer.bat [gtest_filter]`로 빌드와 실행을 한다.
- `Testing/Runtime/DxRender-Test.slnx` — Render 백엔드만 테스트하는 솔루션이다. 가끔 쓴다.
- `Testing/Runtime/FileAsset-Test.slnx` — 에셋 로딩/저장(FileAsset)만 테스트하는 솔루션이다 (Core, Contract, Serialization, Service 포함). 실행 파일은 `Testingd\Debug\FileAsset-Test.exe`다.
- `Testing/Runtime/Runtime.NoRender-Test.slnx` — 렌더를 뺀 런타임 솔루션이다. 당분간 쓰지 않으며, 현재 `Service`/`GoogleTest`에서 pch 오류(C1010)가 난다. 빌드/테스트에 사용하지 않는다.

# 빌드와 실행

- 수동 빌드: `"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\amd64\MSBuild.exe" Testing\Game\UserInterface-Test.slnx -p:Configuration=Debug -p:Platform=x64 -m -v:m -nologo` → 실행 파일은 `Testing\Game\x64\Debug\UserInterface-Test.exe`.
- **직접 `.vcxproj`를 빌드하지 않는다.** 솔루션마다 중간/출력 폴더가 달라서 Visual Studio가 쓰는 결과물을 갱신하지 못한다. 항상 `.slnx`를 위의 64비트(`amd64`) MSBuild로 빌드한다 (32비트는 VS와 다른 컴파일러를 써서 번갈아 빌드하면 전체가 다시 컴파일된다).
- 테스트는 GoogleTest(`Testing/GoogleTest`, 소스 포함) 실행 파일이다. 단일 테스트는 `UserInterface-Test.exe --gtest_filter=UserInterfaceT_Toggle.*` 처럼 실행한다.
- `Clean_x64_vs.bat` — 모든 `x64` 출력 폴더와 `.vs`를 삭제한다.
- 공통 컴파일 옵션/include 경로는 `Directory.Build.props`에 있다. include 루트는 저장소 루트, `Foundation`, `Game`, `Runtime`, `ThirdParty`다.
