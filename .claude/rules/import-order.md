---
paths:
  - "**/*.ixx"
  - "**/*.cpp"
---

# import 순서

위에서 아래로 `import std;` → 같은 모듈의 파티션(`:Widget`) → 가까운 계층부터 먼 계층 순으로 다른 모듈 → `Core`. 같은 묶음 안의 순서는 따로 정하지 않는다. 묶음 사이에 빈 줄은 넣지 않는다. 구현 파일(`.cpp`)은 `module ...;` 다음에 같은 순서를 따른다.

1. `import std;`
2. 같은 모듈의 파티션 (`import :Widget;`)
3. 게임 쪽 모듈 (`UserInterface.*` 등 클라이언트/UI/서비스)
4. `Contract.*`
5. 런타임 백엔드 (`DxRender`, `DxInput`, `FileAsset`, `SDLAudio`, `Serialization`)
6. `Core.*`

순서의 근거는 의존 방향이다 (게임 클라이언트 → 서비스 → Contract ← 백엔드, 가장 아래가 Core). 새 `import`를 추가할 때는 `ModuleGraph.txt`로 방향을 깨지 않는지도 확인한다.
