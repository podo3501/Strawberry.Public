module;

#include "d3dx12.h"

export module DxRender.Core:DxResource;

export class Resource
{
public:
	~Resource() = default;
	Resource() = default;
	Resource(const Resource&) = default;
	Resource& operator=(const Resource&) = default;
	Resource(Resource&&) noexcept = default; // 이동하면 원본은 빈 상태가 된다
	Resource& operator=(Resource&&) noexcept = default;
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