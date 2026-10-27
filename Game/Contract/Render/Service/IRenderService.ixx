module;

#include <Windows.h>

export module Contract.Render.Service:IRenderService;

import :IFrameRecorder;
import Contract.Render.IRenderResources;
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
	virtual void Render() = 0;
	virtual void Resize(const Core::Size& size) = 0;

	virtual IRenderResources& GetResources() = 0;
	virtual IFrameRecorder& GetFrameRecorder() = 0;
	virtual RenderMetrics GetRenderMetrics() = 0;
};

export std::unique_ptr<IRenderService> CreateRenderService(	
	std::unique_ptr<IRenderBackend> backend,
	IAssetAsyncLoader* asyncLoader) noexcept;