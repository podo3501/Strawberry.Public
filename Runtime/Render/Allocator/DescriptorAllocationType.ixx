export module Runtime.Render.Allocator:DescriptorAllocationType;

export enum class DescriptorAllocationType
{
	Persistent,
	Transient,
	Dynamic
};