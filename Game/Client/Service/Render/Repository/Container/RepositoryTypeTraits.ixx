export module Client.Render.Repository:TypeTraits;

import std;
import :ResourceRepositories;
import :RepositoryTypes;

export template <typename TRepo>
struct RepositoryTypeOf;

template <>
struct RepositoryTypeOf<FontRepository>
{
    static constexpr RepositoryType value = RepositoryType::Font;
};

template <>
struct RepositoryTypeOf<MeshRepository>
{
    static constexpr RepositoryType value = RepositoryType::Mesh;
};

template <>
struct RepositoryTypeOf<MaterialRepository>
{
    static constexpr RepositoryType value = RepositoryType::Material;
};

template <>
struct RepositoryTypeOf<DebugMeshRepository>
{
    static constexpr RepositoryType value = RepositoryType::DebugMesh;
};

template <>
struct RepositoryTypeOf<DebugMaterialRepository>
{
    static constexpr RepositoryType value = RepositoryType::DebugMaterial;
};

template <>
struct RepositoryTypeOf<BrushRepository>
{
    static constexpr RepositoryType value = RepositoryType::Brush;
};

template <>
struct RepositoryTypeOf<EnvironmentRepository>
{
    static constexpr RepositoryType value = RepositoryType::Environment;
};