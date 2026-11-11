---
paths:
  - "**/*.ixx"
  - "**/*.cpp"
---

# 파일 경로 사용 범위

- **`std::filesystem::path`는 다음 세 곳에서만 쓴다.**
  1. `Foundation/Core`: 경로 타입과 유틸 (`AssetPath`, `ResourceID`, `PathUtils`)
  2. `Runtime/FileAsset`: 실제 파일을 열고 폴더를 도는 입출력 구현 (`AssetStorage`, `ReadStream`)
  3. 실행 진입점, `Testing/`: 루트 폴더 계산, 폴더 생성, 임시 폴더처럼 실제 파일 시스템을 만지는 곳
- **그 밖에서는 `Core::AssetPath`를 쓴다.** `Game`(Service, UserInterface, Client, App), `DxRender`/`DxInput`/`SDLAudio`, `Foundation/Contract`의 인터페이스, `Foundation/Serialization`이 해당한다. 파일은 저장소 루트 기준 상대 경로(`AssetPath`)로만 가리키고, 루트와 합쳐 실제 경로를 만드는 일은 저장소 구현(`AssetStorage`) 안에서만 한다.
- **유일한 예외**: `Contract`의 `CreateAssetStorage(root)`는 저장소 루트의 실제 OS 경로를 받는다. 저장소를 만드는 쪽이 진입점이라서 `path`를 그대로 받는다.
- 이 규칙을 지키려고 `Core`에 파일 입출력을 옮기지 않는다. `Core`는 아무 데도 의존하지 않는 기반이므로 경로 타입과 유틸까지만 둔다.
- `AssetPath`의 규칙(상대 경로만, 대소문자 보존하되 비교와 해시는 무시, 암묵 변환 없음, 확장자 불필요)은 `ui-module.md`의 "파일 이름은 `Core::AssetPath`" 항목과 `Core/Type/AssetPath.ixx`의 주석을 따른다.
- 소스와 에셋 경로는 UTF-8이다. `std::filesystem::path`의 narrow 변환은 시스템 코드 페이지를 쓰므로 한글 이름이 깨지거나 예외가 난다. 경로와 문자열을 오갈 때는 `Core::PathFromUtf8`/`PathToUtf8`(구분자는 `/`)을 쓰고, `PathFromUtf8`에는 `IsValidUtf8`인 문자열만 넘긴다.
- 새 코드에서 `path`가 필요해 보이면 먼저 위 세 곳에 해당하는지 확인한다. 해당하지 않는데 파일 이름이 필요하면 `AssetPath`로 해결할 수 있는지 본다.
