module;

#include <windows.h>

export module Contract.Render.Backend:IRenderBackend;

import std;
import :IResourceProvider;
import Core.Math;
import Contract.Render.Backend.Types;
import Contract.Render.Backend.View;
import Contract.Render.Shared;

export struct IRenderBackend
{
    virtual ~IRenderBackend() = default;

    // hwnd가 nullptr이면 창 없이 텍스처에만 그리는 오프스크린 모드로 동작한다 (결과는 ReadbackFrame으로 읽는다).
    virtual bool Initialize(
        HWND hwnd, 
        const Core::Size& wndSize, 
        const std::vector<RegistryShaderDesc>& registryShaders) = 0;

    virtual ShaderID RegisterShader(const ShaderDesc& desc) = 0;
    virtual void Resize(const Core::Size& size) = 0;
    virtual void Update() = 0;
    virtual void Render(SceneFrameData frame) = 0;
    virtual void Shutdown() = 0;
    virtual IResourceProvider* GetProvider(ProviderType type) = 0; // 리소스 로딩/관리용 프로바이더 조회

    virtual Core::Vector2 MeasureText(
        const std::shared_ptr<IResource>& font,
        const std::vector<TextRun>& runs,
        std::uint32_t size,
        float maxWidth,
        const TextLayout& layout) = 0;
    virtual RenderMetrics GetRenderMetrics() = 0;
    virtual bool ReadbackFrame(FrameImage& out) = 0; // 오프스크린 모드에서 마지막으로 그린 프레임을 CPU로 읽는다
};

export std::unique_ptr<IRenderBackend> CreateRenderBackend(const RenderConfig& config);