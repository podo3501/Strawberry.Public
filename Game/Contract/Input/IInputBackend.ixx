export module Contract.Input.Interfaces:IInputBackend;

import std;

export struct IInputBackend
{
	virtual ~IInputBackend() = default;
};

export std::unique_ptr<IInputBackend> CreateInputBackend();