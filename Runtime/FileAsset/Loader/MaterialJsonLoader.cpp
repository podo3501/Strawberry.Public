#include <nlohmann/json.hpp>

import std;
import Core.Type;
import Contract.Common.Asset;
import Contract.Render.Assets;
import Contract.Asset.Service;
import Contract.Asset.Backend;
import Serialization;
import Core.Logger;

struct JsonPBRSurface
{
    float normalScale = 1.f;
    float roughnessScale = 1.f;
    float metallicScale = 1.f;
    float aoStrength = 1.f;

    void Serialize(Serializer& serializer)
    {
        serializer.Process("normalScale", normalScale);
        serializer.Process("roughnessScale", roughnessScale);
        serializer.Process("metallicScale", metallicScale);
        serializer.Process("aoStrength", aoStrength);
    }
};

struct JsonPhongSurface
{
    float normalScale = 1.f;
    float ambientScale = 0.1f;
    float specularScale = 0.2f;
    float shininess = 8.f;

    void Serialize(Serializer& serializer)
    {
        serializer.Process("normalScale", normalScale);
        serializer.Process("ambientScale", ambientScale);
        serializer.Process("specularScale", specularScale);
        serializer.Process("shininess", shininess);
    }
};

struct JsonMaterialTextures
{
    std::string albedo;
    std::string normal;
    std::string arm;

    void Serialize(Serializer& serializer)
    {
        serializer.Process("albedo", albedo);
        serializer.Process("normal", normal);
        serializer.Process("arm", arm);
    }
};

struct JsonMaterialHeader // type만 먼저 확인하기 위한 헤더용 구조체
{
    std::string type;

    void Serialize(Serializer& serializer)
    {
        serializer.Process("type", type);
    }
};

// --- Static Loader Helpers ---

// 이름이 비어 있으면 텍스처가 없다는 뜻이라 nullptr이다. 이름이 있는데 올바르지 않거나 읽지 못하면 Error를 남기고 failed를 켠다
static std::shared_ptr<TextureAsset> LoadTexture(
    IAssetProvider* provider,
    const Core::ResourceID& materialID,
    const std::string& name,
    const char* key,
    bool& failed)
{
    if (name.empty()) return nullptr;

    const auto id = materialID.TryMakeSibling(name);
    if (!id)
    {
        Core::Logger::Error("MaterialJsonLoader: {}의 파일 이름이 올바르지 않음 ({})", key, name);
        failed = true;
        return nullptr;
    }

    auto texture = Core::Cast<TextureAsset>(provider->Load(Core::GetTypeID<TextureAsset>(), *id));
    if (!texture)
    {
        Core::Logger::Error("MaterialJsonLoader: {}의 텍스처를 읽지 못함 ({})", key, id->GetValue());
        failed = true;
    }
    return texture;
}

static void LoadCommonTextures(
    MaterialAsset& material,
    const Core::ResourceID& materialID,
    IAssetProvider* provider,
    const JsonMaterialTextures& textures,
    bool& failed)
{
    material.albedo = LoadTexture(provider, materialID, textures.albedo, "albedo", failed);
    material.normal = LoadTexture(provider, materialID, textures.normal, "normal", failed);
}

static std::shared_ptr<MaterialAsset> LoadPBR(
    const Core::ResourceID& materialID,
    const nlohmann::json& data,
    IAssetProvider* provider,
    const JsonMaterialTextures& textures)
{
    JsonPBRSurface surface;
    if (data.contains("surface"))
    {
        auto reader = Serializer::ForRead(data["surface"]);
        surface.Serialize(reader);
        if (reader.HasError())
        {
            Core::Logger::Error("MaterialJsonLoader: surface를 읽지 못함");
            return nullptr;
        }
    }

    auto material = std::make_shared<PbrMaterialAsset>();

    material->type = MaterialType::PBR;
    material->surface.normalScale = surface.normalScale;
    material->surface.roughnessScale = surface.roughnessScale;
    material->surface.metallicScale = surface.metallicScale;
    material->surface.aoStrength = surface.aoStrength;

    bool failed = false;
    LoadCommonTextures(*material, materialID, provider, textures, failed);
    material->arm = LoadTexture(provider, materialID, textures.arm, "arm", failed);

    return failed ? nullptr : material;
}

static std::shared_ptr<MaterialAsset> LoadPhong(
    const Core::ResourceID& materialID,
    const nlohmann::json& data,
    IAssetProvider* provider,
    const JsonMaterialTextures& textures)
{
    JsonPhongSurface surface;
    if (data.contains("surface"))
    {
        auto reader = Serializer::ForRead(data["surface"]);
        surface.Serialize(reader);
        if (reader.HasError())
        {
            Core::Logger::Error("MaterialJsonLoader: surface를 읽지 못함");
            return nullptr;
        }
    }

    auto material = std::make_shared<PhongMaterialAsset>();

    material->type = MaterialType::Phong;
    material->surface.normalScale = surface.normalScale;
    material->surface.ambientScale = surface.ambientScale;
    material->surface.specularScale = surface.specularScale;
    material->surface.shininess = surface.shininess;

    bool failed = false;
    LoadCommonTextures(*material, materialID, provider, textures, failed);

    return failed ? nullptr : material;
}

// --- Loader Class implementation ---

class MaterialJsonLoader : public IAssetLoader
{
public:
    ~MaterialJsonLoader() override = default;

    explicit MaterialJsonLoader(IAssetProvider* assetProvider) noexcept
        : m_assetProvider{ assetProvider }
    {
    }

    std::shared_ptr<Asset> Load(AssetInput& source) override
    {
        if (source.IsStream()) return nullptr;

        auto& mem = static_cast<MemoryInput&>(source);
        return LoadFromMemory(mem.resID, std::move(mem.buffer));
    }

private:
    std::shared_ptr<MaterialAsset> LoadFromMemory(
        const Core::ResourceID& resID,
        Core::ByteBuffer buffer)
    {
        nlohmann::json data = nlohmann::json::parse(buffer.begin(), buffer.end(), nullptr, false);   // 예외 없이 실패를 알린다
        if (data.is_discarded())
        {
            Core::Logger::Error("MaterialJsonLoader: JSON 문법이 올바르지 않음");
            return nullptr;
        }

        JsonMaterialHeader header;
        auto reader = Serializer::ForRead(data);
        header.Serialize(reader);
        if (reader.HasError())
        {
            Core::Logger::Error("MaterialJsonLoader: 머티리얼 머리 항목을 읽지 못함");
            return nullptr;
        }

        JsonMaterialTextures textures;
        if (data.contains("textures"))
        {
            auto texturesReader = Serializer::ForRead(data["textures"]);
            textures.Serialize(texturesReader);
            if (texturesReader.HasError())
            {
                Core::Logger::Error("MaterialJsonLoader: textures를 읽지 못함");
                return nullptr;
            }
        }

        if (header.type == "PBR")
            return LoadPBR(resID, data, m_assetProvider, textures);
        if (header.type == "Phong")
            return LoadPhong(resID, data, m_assetProvider, textures);

        Core::Logger::Error("MaterialJsonLoader: 알 수 없는 type ({}). PBR이나 Phong이어야 함", header.type);
        return nullptr;
    }

private:
    IAssetProvider* m_assetProvider{ nullptr };
};

std::unique_ptr<IAssetLoader> CreateMaterialJsonLoader(IAssetProvider* assetProvider)
{
    return std::make_unique<MaterialJsonLoader>(assetProvider);
}
