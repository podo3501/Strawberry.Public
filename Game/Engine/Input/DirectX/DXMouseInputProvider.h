#pragma once
#include "Client/Service/Input/IMouseInputProvider.h"
#include "DirectXTK12/Mouse.h"

class DXMouseInputProvider : public IMouseInputProvider
{
public:
	~DXMouseInputProvider();
	DXMouseInputProvider(HWND hwnd) noexcept;
	virtual void Update() noexcept override;
	virtual const MouseState& GetState() const noexcept override;

private:
	enum class UpdateMode { Sync, Normal };
	void UpdateButton(const DirectX::Mouse::State& dxState, UpdateMode mode);

	static DirectX::Mouse m_mouse;
	bool m_initialized{ false };
	MouseState m_state{};
};