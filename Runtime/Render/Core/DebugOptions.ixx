export module Runtime.Render.Core:DebugOptions;

export struct DebugOptions
{
	bool enableDebugLayer{ true };
	bool enableGpuValidation{ false };
	bool breakOnWarning{ false };
};