module;

#include <wrl/client.h>
#include <d3d12sdklayers.h>

export module DxRender.Core:DebugHelper;

import std;
import :DebugOptions;

export class DebugHelper
{
public:
	static void EnableDebugLayer(const DebugOptions& opt)
	{
#if defined(_DEBUG)
		if (!opt.enableDebugLayer)
			return;

		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			return;

		debugController->EnableDebugLayer();

		if (opt.enableGpuValidation)
		{
			Microsoft::WRL::ComPtr<ID3D12Debug1> debugController1;
			if (SUCCEEDED(debugController.As(&debugController1)))
			{
				debugController1->SetEnableGPUBasedValidation(TRUE);
			}
		}
#endif
	}

	static void SetupInfoQueue(ID3D12Device* device, const DebugOptions& opt)
	{
#if defined(_DEBUG)
		if (!device)
			return;

		Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue;
		if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
		{
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

			if (opt.breakOnWarning)
			{
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
			}

			// 선택: noisy 메시지 필터링
			D3D12_MESSAGE_ID denyIds[] =
			{
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
			};

			D3D12_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = static_cast<UINT>(std::size(denyIds));
			filter.DenyList.pIDList = denyIds;

			infoQueue->AddStorageFilterEntries(&filter);
		}
#endif
	}
};