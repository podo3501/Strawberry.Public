module;

#include <windows.h>

export module Client.Render.Interfaces:IRenderBackend;

import std;
import :IResourceProvider;
import Core.Math;
import Client.Render.Definition;
import Client.Render.View;

export struct IRenderBackend
{
    virtual ~IRenderBackend() = default;

    virtual bool Initialize(HWND hwnd, const Core::Size& wndSize, std::span<const RegistryShaderDesc> registryShaders) = 0;
    virtual ShaderID RegisterShader(const ShaderDesc& desc) = 0;
    virtual void Resize(const Core::Size& size) = 0;
    virtual void Update() = 0;
    virtual void Render(SceneFrameData frame) = 0;
    virtual void Shutdown() = 0;

    virtual IResourceProvider* GetProvider(ProviderType type) = 0; // 리소스 로딩/관리용 프로바이더 조회
    virtual RenderMetrics GetRenderMetrics() = 0;
};

export std::unique_ptr<IRenderBackend> CreateRenderBackend(const RenderConfig& config);