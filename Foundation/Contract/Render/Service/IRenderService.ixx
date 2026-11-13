module;

#include <Windows.h>

export module Contract.Render.Service:IRenderService;

import :IFrameRecorder;
import Contract.Render.IRenderResources;
import Contract.Render.ITextMeasurer;
import Contract.Render.Shared;
import Contract.Render.Backend;
import Contract.Common.IAssetAsyncLoader;
import Core.Type;
import Core.Math;

export struct IRenderService
{
	virtual ~IRenderService() = default;

	virtual bool Initialize(HWND hwnd, const Core::Size& screenSize) = 0;
	virtual ShaderID RegisterShader(const Core::ResourceID& resourceID, ShaderType type) = 0;

	virtual void Update() = 0;
	virtual bool Render() = 0; // 그리지 못했거나 화면에 내보내지 못했으면 false (원인은 백엔드가 Error로 남긴다)
	virtual bool Resize(const Core::Size& size) = 0; // 실패하면 다시 Resize할 때까지 프레임을 그리지 못한다

	virtual IRenderResources& GetResources() = 0;
	virtual IFrameRecorder& GetFrameRecorder() = 0;
	virtual ITextMeasurer& GetTextMeasurer() = 0;
	virtual RenderMetrics GetRenderMetrics() = 0;
	virtual bool ReadbackFrame(FrameImage& out) = 0; // 오프스크린 모드에서 마지막으로 그린 프레임을 CPU로 읽는다
};

export std::unique_ptr<IRenderService> CreateRenderService(	
	std::unique_ptr<IRenderBackend> backend,
	IAssetAsyncLoader* asyncLoader) noexcept;