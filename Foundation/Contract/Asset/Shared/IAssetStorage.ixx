export module Contract.Asset.Shared:IAssetStorage;

import std;
import :IReadStream;
import Core.AssetPath;
import Core.BasicTypes;

// 저장소 루트 아래의 파일을 읽고 쓴다. 파일은 루트 기준 상대 경로(AssetPath)로 가리킨다.
export struct IAssetStorage
{
	virtual ~IAssetStorage() = default;

	virtual bool Write(const Core::AssetPath& filename, const Core::ByteBuffer& buffer) noexcept = 0;
	virtual bool Read(const Core::AssetPath& filename, Core::ByteBuffer& outBuffer) const = 0;
	virtual bool WriteText(const Core::AssetPath& filename, const std::string& text) noexcept = 0;
	virtual bool ReadText(const Core::AssetPath& filename, std::string& outText) const noexcept = 0;

	// 루트 아래(하위 폴더 포함)에서 확장자가 extension인 파일을 모두 찾는다. extension은 ".meta"처럼 점을 포함하고 대소문자를 구분한다.
	// 순서는 정해지지 않는다. 루트가 없거나 읽을 수 없으면 빈 목록
	virtual std::vector<Core::AssetPath> FindFiles(std::string_view extension) const = 0;

	virtual std::unique_ptr<IReadStream> CreateReadStream(const Core::AssetPath& filename) = 0;
};

export std::unique_ptr<IAssetStorage> CreateAssetStorage(const std::filesystem::path& path);   // path: 저장소 루트 (절대 경로여도 된다)
