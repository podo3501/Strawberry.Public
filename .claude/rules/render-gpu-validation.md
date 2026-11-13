---
paths:
  - "Runtime/DxRender/**"
  - "Testing/Runtime/DxRender-Test/**"
---

# GPU 기반 검증 (렌더 코드를 고칠 때)

- **기본은 꺼져 있다.** 검증을 켜면 렌더 Agent 시험이 약 3배(약 20초 -> 약 60초) 느려지고, 지금까지(Agent 시험 1015개) 이 검증이 잡은 문제는 없었다. 매번 켜지 않는다.
- **작업하는 동안에는 필터만 쓴다.** 고친 코드에 딱 맞는 시험 묶음만 `Test_Render.bat "<필터>"`로 돌린다 (검증 없이).
- **한 덩어리 작업이 끝나 빌드와 시험이 통과하면 한 번만 검증을 켜서 돌린다** (`Test_Render.bat "<필터>" gbv`). 범위는 아래 표에서 고르고, 영역이 걸쳐 있거나 판단이 서지 않으면 전체를 `gbv`로 한 번 돌린다 (약 1분). 돌린 범위와 이유와 결과(레이어 오류가 있었는지)를 사용자에게 알린다.
- **검증을 생략하는 작업**: 문서, 규칙, Service/Game 쪽 시험, 시험 정리처럼 렌더 코드나 셰이더를 건드리지 않은 작업. 렌더 코드나 셰이더를 고쳤다면 생략하지 않는다.
- 마지막 한 번에서 오류가 나오면 그전에 통과한 시험이 틀렸던 것이 아니라 GPU 쪽에서만 보이는 문제가 새로 드러난 것이므로, 직전에 고친 범위 안에서 원인을 찾는다.
- User 테스트(진짜 GPU의 보이는 창)에는 검증을 켤 수 없다 (켜면 디버그 레이어 안에서 죽는다).

| 고친 영역 | 마지막에 `gbv`로 돌릴 묶음 (필터 예) |
|---|---|
| 바인드리스 서술자와 힙 할당기 (`Allocator/*`, `DescriptorFactory`) | `*Allocator*:*Descriptor*:*Renderer*:*GraphBuilder*` |
| 리소스와 프로바이더 (`Resource/*`, `Provider/*`, 메시 버퍼, 텍스처, 환경) | `*Provider*:*Resource*:*GraphBuilder*:*Renderer*` |
| 셰이더, 렌더러, 파이프라인, 그래프 (`Shader`, `Pipeline/*`, `Graph/*`) | `*Renderer*:*GraphBuilder*:*Graph*:RenderBackend*:*Shader*` |
| 명령, 스왑 체인, 백엔드 (`Command/*`, `SwapChainPresenter`, `RenderBackend`) | `*Command*:SwapChain*:RenderBackend*` |
