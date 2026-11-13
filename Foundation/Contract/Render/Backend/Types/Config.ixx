export module Contract.Render.Backend.Types:Config;

import std;
import Core.Math;

export struct CommandPoolConfig
{
    std::uint32_t direct{ 30 };
    std::uint32_t copy{ 4 };
    std::uint32_t compute{ 4 };
};

export struct DescriptorConfig
{
    std::uint32_t rtvCount{ 64 }; // 할당 후 해제 가능하므로 64면 충분
    std::uint32_t dsvCount{ 64 }; // 주로 내부 Frame Resource 용도이므로 64면 충분
};

export struct BitmapConfig
{
    Core::Size atlasSize{ 256, 256 }; // 테스트용 크기 (일반적으로 1024/2048 사용)
};

export struct MTSDFConfig
{
    Core::Size atlasSize{ 512, 512 }; // 테스트용 크기 (일반적으로 1024/2048 사용)
};

export struct TextConfig
{
    BitmapConfig bitmap;
    MTSDFConfig mtsdf;
};

export enum class RenderAdapter
{
    Auto, // 하드웨어 어댑터를 먼저 시도하고 없으면 WARP
    Warp  // 소프트웨어 렌더러로 고정 (결과가 기계마다 같다)
};

export struct RenderConfig
{
    bool enableDebugLayer{ true };
    bool allowTearing{ true };
    RenderAdapter adapter{ RenderAdapter::Auto };

    CommandPoolConfig commandPools;
    DescriptorConfig descriptors;
    TextConfig text;
};