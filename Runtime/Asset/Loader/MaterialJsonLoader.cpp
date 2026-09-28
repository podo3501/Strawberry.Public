#include <nlohmann/json.hpp>

import std;
import Core.Types;
import Core.TypeHierarchy;
import Core.ResourceID;
import Client.Asset.Contract;
import Client.Asset.AssetData;
import Runtime.Serialization;

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

static std::shared_ptr<TextureAsset> LoadTexture(
    IAssetProvider* provider,
    const Core::ResourceID& materialID,
    const std::string& path)
{
    if (path.empty()) return nullptr;

    auto asset = provider->Load(Core::GetTypeID<TextureAsset>(), materialID.MakeSibling(path));
    return Core::Cast<TextureAsset>(asset);
}

static void LoadCommonTextures(
    MaterialAsset& material,
    const Core::ResourceID& materialID,
    IAssetProvider* provider,
    const JsonMaterialTextures& textures)
{
    material.albedo = LoadTexture(provider, materialID, textures.albedo);
    material.normal = LoadTexture(provider, materialID, textures.normal);
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
        Serializer reader{ std::as_const(data["surface"]) };
        surface.Serialize(reader);
    }

    auto material = std::make_shared<PbrMaterialAsset>();

    material->type = MaterialType::PBR;
    material->surface.normalScale = surface.normalScale;
    material->surface.roughnessScale = surface.roughnessScale;
    material->surface.metallicScale = surface.metallicScale;
    material->surface.aoStrength = surface.aoStrength;

    LoadCommonTextures(*material, materialID, provider, textures);
    material->arm = LoadTexture(provider, materialID, textures.arm);

    return material;
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
        Serializer reader{ std::as_const(data["surface"]) };
        surface.Serialize(reader);
    }

    auto material = std::make_shared<PhongMaterialAsset>();

    material->type = MaterialType::Phong;
    material->surface.normalScale = surface.normalScale;
    material->surface.ambientScale = surface.ambientScale;
    material->surface.specularScale = surface.specularScale;
    material->surface.shininess = surface.shininess;

    LoadCommonTextures(*material, materialID, provider, textures);

    return material;
}

// --- Loader Class implementation ---

class MaterialJsonLoader : public IAssetLoader
{
public:
    virtual ~MaterialJsonLoader() override = default;

    explicit MaterialJsonLoader(IAssetProvider* assetProvider) noexcept
        : m_assetProvider{ assetProvider }
    {
    }

    virtual std::shared_ptr<AssetData> Load(AssetInput& source) override
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
        nlohmann::json data = nlohmann::json::parse(buffer.begin(), buffer.end());

        JsonMaterialHeader header;
        Serializer reader{ std::as_const(data) };
        header.Serialize(reader);

        JsonMaterialTextures textures;
        if (data.contains("textures"))
        {
            Serializer reader{ std::as_const(data["textures"]) };
            textures.Serialize(reader);
        }

        if (header.type == "PBR")
            return LoadPBR(resID, data, m_assetProvider, textures);

        return LoadPhong(resID, data, m_assetProvider, textures);
    }

private:
    IAssetProvider* m_assetProvider{ nullptr };
};

std::unique_ptr<IAssetLoader> CreateMaterialJsonLoader(IAssetProvider* assetProvider)
{
    return std::make_unique<MaterialJsonLoader>(assetProvider);
}
