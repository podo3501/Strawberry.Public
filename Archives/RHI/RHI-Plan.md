# RHI 도입 계획

## 목표

- DxRender에서 D3D12를 RHI라는 경계 뒤로 숨긴다.
- 구현은 D3D12 하나만 유지한다. Vulkan 구현은 필요해질 때 RHI 구현만 새로 만든다.
- 렌더러(Graph, Pipeline, Provider, Text)는 RHI 타입과 함수만 쓴다. `d3d12.h`는 RHI 구현 안에만 남긴다.
- 화면 결과와 시험(`Test_Render.bat`의 Agent 시험)은 모든 단계에서 그대로 통과해야 한다.

## 설계 원칙 (Vulkan 기준으로 맞춘다)

- **배리어**: `D3D12_RESOURCE_STATES` 대신 layout/access/sync 형태의 추상 상태를 쓴다 (D3D12 Enhanced Barriers, Vulkan에 모두 대응).
- **바인딩**: 전역 바인드리스 레이아웃 하나와 push constant(루트 상수)로 통일한다. 루트 시그니처, 서술자 힙은 RHI 밖으로 드러내지 않는다.
- **파이프라인**: `PipelineDesc` 하나로 만든다. PSO와 루트 시그니처를 직접 만지지 않는다.
- **GPU 주소**: `D3D12_GPU_VIRTUAL_ADDRESS` 대신 RHI 버퍼 주소 타입을 쓴다.
- **셰이더**: HLSL을 유지하고, DXC로 DXIL을 만든다. 나중에 SPIR-V도 뽑을 수 있게 register/space 규칙만 정해 둔다.
- **좌표계**: 뷰포트 Y 뒤집기 같은 차이는 RHI 구현 안에서 흡수한다.
- **백엔드 선택**: 빌드할 때 고른다 (가상 함수 없이 구체 클래스). 같은 모듈 이름을 구현만 다르게 둔다.
- **프로젝트 분리**: 처음에는 DxRender 안에서 모듈 경계만 나눈다 (`RHI`, `RHI.D3D12`). 6단계에서 별도 프로젝트로 옮긴다.

## 진행 원칙

- 한 단계가 끝날 때마다 빌드와 `Test_Render.bat`가 통과하는 상태를 유지한다.
- 큰 단계는 파일(Renderer 등) 하나씩 옮기고 시험한다.
- 단계가 끝나면 GPU 기반 검증(`Test_Render.bat "<필터>" gbv`)을 한 번 돌린다 (범위는 `.claude/rules/render-gpu-validation.md`의 표).
- 단계가 끝나면 `ModuleGraph.txt`를 새로 뽑아 의존 방향과 순환을 확인한다.
- 커밋은 단계마다 하나씩 해서 따로 되돌릴 수 있게 한다.

## 현재 상황 (시작 전 조사)

- `IRenderBackend`(`Foundation/Contract/Render/Backend/IRenderBackend.ixx`)에는 D3D12 타입이 없다. Service/Game 쪽은 고칠 필요가 없다.
- D3D12 타입이 보이는 파일은 `d3dx12.h`를 빼고 64개다.
- 가장 큰 구멍은 `CommandList::operator->`(`Command/CommandList.ixx`)로, `ID3D12GraphicsCommandList*`를 그대로 돌려준다.
- 그 밖에 많이 쓰는 곳:
  - Renderer의 PSO와 루트 시그니처: `SurfaceRenderer`, `PipelineCache`, `RootSignatureBuilder`
  - 상수 버퍼 주소: `D3D12_GPU_VIRTUAL_ADDRESS`
  - 렌더 그래프 배리어: `BarrierBuilder`
  - 서술자: `BindlessDescriptorAllocator`, `DescriptorFactory`
  - `MipGenerator`, `TextureUtils`, `SwapChainPresenter`
- 이미 API와 무관한 부분: `Task`, `RGResourceID`, `Packet`, `Text` 레이아웃, `Graph`의 대부분.

## 단계

### 1단계: RHI 기본 타입

- D3D 헤더 없이 Format, 리소스 상태(layout/access/sync), `BufferDesc`, `TextureDesc`, `PipelineDesc`, 핸들 타입을 정의한다.
- 흩어진 변환 코드(`Helper/D3D12Conversions`, `Command/Conversions`, `Definition/RenderStateTypes`)를 D3D12 구현 쪽 변환 함수로 모은다.
- 기존 코드는 그대로 두고 타입과 변환만 추가한다.
- 확인: 빌드, `Test_Render.bat`.

### 2단계: CommandList 감싸기

- `operator->`와 `Get()`을 없애고 `Draw`, `SetVertexBuffer`, `Barrier`, `Copy`, `SetConstants` 같은 명시적 메서드로 바꾼다.
- Renderer를 하나씩 옮긴다 (`SurfaceRenderer`, `ShadowRenderer`, `SkyboxRenderer`, `UIRenderer`, `DebugSurfaceRenderer`, `CompositeRenderer`, Inspector, `MipGenerator`, `FontAtlasUploadGraphBuilder` 등). 하나 옮길 때마다 시험한다.
- 이 단계가 가장 크다. 옮기는 도중에도 옛 호출과 새 호출이 함께 있을 수 있게 한다.
- 확인: 빌드, `Test_Render.bat`, `gbv` (명령, 렌더러 묶음).

### 3단계: PSO와 루트 시그니처를 PipelineDesc로

- `PipelineCache`와 각 Renderer의 `CreatePSO`를 `PipelineDesc` 기반으로 바꾼다.
- `RootSignatureBuilder`를 RHI 구현 안으로 숨기고, 바인드리스 전역 레이아웃 하나로 통일한다.
- 상수 버퍼의 `D3D12_GPU_VIRTUAL_ADDRESS`를 RHI 버퍼 주소 타입으로 바꾼다.
- 확인: 빌드, `Test_Render.bat`, `gbv` (셰이더, 렌더러, 파이프라인 묶음).

### 4단계: 렌더 그래프 배리어를 추상 상태로

- `BarrierBuilder`와 `Graph`의 리소스 상태를 1단계의 추상 상태로 바꾼다.
- D3D12 쪽에서는 추상 상태를 `D3D12_RESOURCE_STATES`(또는 Enhanced Barriers)로 변환한다.
- 배리어가 틀리면 컴파일러가 잡지 못하므로 이 단계는 `gbv`를 꼭 돌린다.
- 확인: 빌드, `Test_Render.bat`, `gbv` (렌더러, 그래프 묶음).

### 5단계: Device, Factory, Allocator, SwapChain을 D3D12 구현으로 옮기기

- `DxDevice`, `ResourceFactory`, `DescriptorFactory`, `BindlessDescriptorAllocator`, `FrameUploadAllocator`, `SwapChainPresenter`를 RHI 구현 모듈로 옮긴다.
- 끝났을 때 상위 계층(Graph, Pipeline, Provider, Text, Packet)에 `#include <d3d12.h>`가 하나도 남지 않아야 한다. `ModuleGraph.txt`로 의존 방향도 확인한다.
- 확인: 빌드, `Test_Render.bat` 전체를 `gbv`로 한 번 (약 1분).

### 6단계: 프로젝트 분리 (선택)

- 모듈 경계가 안정되면 `Runtime/RHI`(타입, 인터페이스)와 `Runtime/RHI.D3D12`(구현)를 별도 프로젝트로 옮긴다.
- `Strawberry.slnx`, `Runtime-Test.slnx`, `.vcxproj`와 `.filters`를 함께 갱신한다.
- 확인: `Test_Render.bat`, `Build_Strawberry.bat` (전체 컴파일).

### 7단계: Vulkan 구현 (필요해질 때)

- RHI의 Vulkan 구현을 새로 만들고 HLSL을 SPIR-V로도 컴파일한다.
- 시험 환경은 lavapipe나 SwiftShader 같은 소프트웨어 구현을 따로 마련한다.
- 렌더러 코드는 거의 건드리지 않는 것이 목표다. 고쳐야 한다면 RHI가 D3D12에 치우쳤다는 신호이므로 RHI를 먼저 고친다.

## 위험과 대응

- **구현이 하나뿐이라 추상화가 맞는지 검증되지 않는다**: Vulkan 기준으로 개념을 잡고(위 설계 원칙), 7단계에서 어긋난 부분을 RHI 쪽에서 고친다.
- **배리어와 리소스 상태 오류는 화면과 GPU 검증에서만 드러난다**: 4단계와 5단계에서 `gbv`를 꼭 돌린다.
- **`.ixx`가 바뀌면 import한 쪽이 모두 다시 컴파일된다**: 단계를 잘게 나누고 변경을 `.cpp` 쪽에 가두어 빌드 시간을 줄인다.
- **공통분모로 깎여 기능이 제한된다**: 한쪽에만 있는 기능은 RHI에 억지로 넣지 않고 구현 안에서 처리한다.
