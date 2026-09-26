export module Client.Asset.Contract:AssetInput;

import std;
import Core.ResourceID;
import Core.Types;
import :IReadStream;

export struct AssetInput
{
	virtual ~AssetInput() = default;
	virtual bool IsStream() const = 0;

	explicit AssetInput(const Core::ResourceID& id) noexcept :
		resID{ id }
	{}

	Core::ResourceID resID;
};

export struct MemoryInput : public AssetInput
{
	Core::ByteBuffer buffer;

	MemoryInput(const Core::ResourceID& resID, std::vector<std::byte>&& buf) noexcept :
		AssetInput{ resID },
		buffer{ std::move(buf) }
	{}

	bool IsStream() const override { return false; }
};

export struct StreamInput : public AssetInput
{
	std::unique_ptr<IReadStream> stream;

	StreamInput(const Core::ResourceID& resID, std::unique_ptr<IReadStream>&& s) noexcept :
		AssetInput{ resID },
		stream{ std::move(s) }
	{}

	bool IsStream() const override { return true; }
};