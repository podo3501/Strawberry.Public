import std;
import Core.Assert;
import Core.Math;
import Contract.Asset;

static bool ParseVector3Line(const std::string& line, Core::Vector3& outVec)
{
    auto openParen = line.find('(');
    auto closeParen = line.find(')');
    if (openParen == std::string::npos || closeParen == std::string::npos || closeParen <= openParen)
        return false;

    std::string inner = line.substr(openParen + 1, closeParen - openParen - 1);

    // 쉼표를 공백으로 바꿔서 스트림으로 편하게 파싱
    for (char& c : inner)
    {
        if (c == ',')
            c = ' ';
    }

    std::istringstream stream(inner);
    float x, y, z;
    if (!(stream >> x >> y >> z))
        return false;

    outVec = Core::Vector3(x, y, z);
    return true;
}

class SphericalHarmonicsLoader : public IAssetLoader
{
public:
    ~SphericalHarmonicsLoader() override = default;

    std::shared_ptr<AssetData> Load(AssetInput& source) override
    {
        if (source.IsStream()) return nullptr;

        auto& mem = static_cast<MemoryInput&>(source);
        return LoadFromMemory(std::move(mem.buffer));
    }

private:
    std::shared_ptr<SphericalHarmonicsAsset> LoadFromMemory(std::vector<std::byte> buffer)
    {
        std::string text(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        std::istringstream stream(text);

        auto asset = std::make_shared<SphericalHarmonicsAsset>();

        std::string line;
        size_t index = 0;

        while (std::getline(stream, line) && index < asset->coefficients.size())
        {
            // 빈 줄이나 괄호가 없는 줄은 무시
            if (line.find('(') == std::string::npos)
                continue;

            Core::Vector3 coeff;
            if (!ParseVector3Line(line, coeff))
            {
                Core::Assert(false); // 예상한 포맷과 다른 줄 -> sh.txt 포맷 확인 필요
                return nullptr;
            }

            asset->coefficients[index] = coeff;
            ++index;
        }

        if (index != asset->coefficients.size())
        {
            Core::Assert(false); // 9개를 못 채움 -> 파일 잘림 또는 포맷 오류
            return nullptr;
        }

        return asset;
    }
};

// 외부 노출 팩토리 함수
std::unique_ptr<IAssetLoader> CreateSphericalHarmonicsLoader()
{
    return std::make_unique<SphericalHarmonicsLoader>();
}