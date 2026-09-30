module;

#include "d3dx12.h"

export module Runtime.Render.Core:DxResource;

export class Resource
{
public:
	~Resource() = default;
	Resource() = default;
	explicit Resource(Microsoft::WRL::ComPtr<ID3D12Resource> resource) noexcept
		: m_resource(std::move(resource))
	{
	}

	explicit operator bool() const noexcept { return m_resource != nullptr; }
	void Reset() noexcept { m_resource.Reset(); }

	ID3D12Resource* operator->() const noexcept { return m_resource.Get(); }
	ID3D12Resource* Get() const noexcept { return m_resource.Get(); }
	ID3D12Resource** GetAddressOf() noexcept { return m_resource.GetAddressOf(); }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
};