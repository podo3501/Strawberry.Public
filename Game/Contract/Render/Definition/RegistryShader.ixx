export module Client.Render.Definition:RegistryShader;

import std;
import :ShaderTypes;

export namespace RegistryShader
{
    // 주의: 이 값은 파일 저장, 직렬화, 네트워크 전송용 값으로 사용하면 안 됩니다.
    // InspectorImage는 Release 빌드 시 제외될 수 있습니다.
    constexpr ShaderID Shadow{ 1 };
    constexpr ShaderID Phong{ 2 };
    constexpr ShaderID PBR{ 3 };
    constexpr ShaderID Grid{ 4 };
    constexpr ShaderID UI{ 5 };
    constexpr ShaderID Skybox{ 6 };
    constexpr ShaderID Composite{ 7 };     // View 합성용 셰이더
    constexpr ShaderID MipGenerator{ 8 };  // Compute 셰이더

    constexpr ShaderID InspectorImage{ 9 }; // 테스트용 (Release 빌드 시 제외될 수 있음)
}