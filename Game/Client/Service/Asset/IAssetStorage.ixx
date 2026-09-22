export module Client.Asset:IAssetStorage;

import std;
import :IReadStream;
import Core.Types;

namespace Client
{
	export struct IAssetStorage
	{
		virtual ~IAssetStorage() = default;

		virtual bool Write(const std::filesystem::path& filename, const Core::ByteBuffer& buffer) noexcept = 0;
		virtual bool Read(const std::filesystem::path& filename, Core::ByteBuffer& outBuffer) const = 0;
		virtual bool WriteText(const std::filesystem::path& filename, const std::string& text) noexcept = 0;
		virtual bool ReadText(const std::filesystem::path& filename, std::string& outText) const noexcept = 0;

		virtual std::filesystem::path MakeAssetFilePath(const std::filesystem::path& filename) const noexcept = 0;
		virtual std::unique_ptr<IReadStream> CreateReadStream(const std::filesystem::path& filename) = 0;
	};

	export std::unique_ptr<IAssetStorage> CreateAssetStorage(const std::filesystem::path& path);
}