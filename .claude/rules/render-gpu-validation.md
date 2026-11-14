---
paths:
  - "Runtime/DxRender/**"
  - "Testing/Runtime/DxRender-Test/**"
---

# GPU 기반 검증 (렌더 코드를 고칠 때)

- **기본은 켜져 있다.** `Test_Render.bat`은 GPU 기반 검증을 켠 채로 돈다. 전체 실행은 약 15초이고 검증을 끄면 약 3초이다 (Agent 테스트 1018개 기준, 프로세스 4개로 나눠 돌 때).
- **끄는 법**: `Test_Render.bat "<필터>" nogbv`. 필터로 몇 개만 돌리는데 느리게 느껴질 때나 검증과 상관없는 작업(문서, 규칙, Service/Game 쪽 테스트, 테스트 정리)에 쓴다.
- **검증을 켠 채로 끝까지 확인한다.** 렌더 코드나 셰이더를 고쳤다면 한 덩어리 작업을 끝낼 때 `nogbv` 없이 마지막으로 한 번 돌린다. 범위는 아래 표에서 고르고, 영역이 걸쳐 있거나 판단이 서지 않으면 필터 없이 전체를 돌린다. 돌린 범위와 이유와 결과(레이어 오류가 있었는지)를 사용자에게 알린다.
- 마지막 한 번에서 오류가 나오면 그전에 통과한 테스트가 틀렸던 것이 아니라 GPU 쪽에서만 보이는 문제가 새로 드러난 것이므로, 직전에 고친 범위 안에서 원인을 찾는다.
- User 테스트(진짜 GPU의 보이는 창)에는 검증을 켤 수 없어서 항상 꺼진다 (켜면 디버그 레이어 안에서 죽는다).

| 고친 영역 | 마지막에 돌릴 묶음 (필터 예) |
|---|---|
| 바인드리스 서술자와 힙 할당기 (`Allocator/*`, `DescriptorFactory`) | `*Allocator*:*Descriptor*:*Renderer*:*GraphBuilder*` |
| 리소스와 프로바이더 (`Resource/*`, `Provider/*`, 메시 버퍼, 텍스처, 환경) | `*Provider*:*Resource*:*GraphBuilder*:*Renderer*` |
| 셰이더, 렌더러, 파이프라인, 그래프 (`Shader`, `Pipeline/*`, `Graph/*`) | `*Renderer*:*GraphBuilder*:*Graph*:RenderBackend*:*Shader*` |
| 명령, 스왑 체인, 백엔드 (`Command/*`, `SwapChainPresenter`, `RenderBackend`) | `*Command*:SwapChain*:RenderBackend*` |
