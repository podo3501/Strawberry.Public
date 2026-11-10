export module Contract.Asset.Shared:IAssetStorage;

import std;
import :IReadStream;
import Core.AssetPath;
import Core.BasicTypes;

// 저장소 루트 아래의 파일을 읽고 쓴다. 파일은 루트 기준 상대 경로(AssetPath)로 가리킨다.
export struct IAssetStorage
{
	virtual ~IAssetStorage() = default;

	// 쓰기는 파일이 놓일 폴더가 없으면 만들고, 임시 파일에 쓴 뒤 이름을 바꿔 덮어쓴다 (실패해도 기존 파일이 남는다)
	// 실패하면 false이고 원인은 Error로 남긴다. 읽기는 없는 파일이면 로그 없이 false이고 outBuffer/outText를 비운다
	virtual bool Write(const Core::AssetPath& filename, const Core::ByteBuffer& buffer) noexcept = 0;
	virtual bool Read(const Core::AssetPath& filename, Core::ByteBuffer& outBuffer) const = 0;
	virtual bool WriteText(const Core::AssetPath& filename, const std::string& text) noexcept = 0;
	virtual bool ReadText(const Core::AssetPath& filename, std::string& outText) const noexcept = 0;

	// 루트 아래(하위 폴더 포함)에서 확장자가 extension인 파일을 모두 찾는다. extension은 ".meta"처럼 점을 포함하고 대소문자를 구분한다.
	// 순서는 정해지지 않는다. 루트가 없거나 읽을 수 없으면 빈 목록
	virtual std::vector<Core::AssetPath> FindFiles(std::string_view extension) const = 0;

	// 파일이 마지막으로 바뀐 시각. 파일이 없거나 읽을 수 없으면 nullopt. 값 자체의 의미보다 "이전에 본 값과 다른가"로 변경을 알아내는 데 쓴다
	virtual std::optional<std::filesystem::file_time_type> GetModifiedTime(const Core::AssetPath& filename) const noexcept = 0;

	virtual std::unique_ptr<IReadStream> CreateReadStream(const Core::AssetPath& filename) = 0;
};

export std::unique_ptr<IAssetStorage> CreateAssetStorage(const std::filesystem::path& path);   // path: 저장소 루트 (절대 경로여도 된다)
