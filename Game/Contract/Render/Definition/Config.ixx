export module Client.Render.Definition:Config;

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

export struct RenderConfig
{
    bool enableDebugLayer{ true };
    bool allowTearing{ true };

    CommandPoolConfig commandPools;
    DescriptorConfig descriptors;
    TextConfig text;
};