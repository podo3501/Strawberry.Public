export module Client.Render.ResourceHandles;

import Core.Handle;

export struct BrushTag {};
export using BrushHandle = Core::IDHandle<BrushTag>;

export struct DebugMaterialTag {};
export using DebugMaterialHandle = Core::IDHandle<DebugMaterialTag>;

export struct DebugMeshTag {};
export using DebugMeshHandle = Core::IDHandle<DebugMeshTag>;

export struct EnvironmentTag {};
export using EnvironmentHandle = Core::IDHandle<EnvironmentTag>;

export struct FontTag {};
export using FontHandle = Core::IDHandle<FontTag>;

export struct MaterialTag {};
export using MaterialHandle = Core::IDHandle<MaterialTag>;

export struct MeshTag {};
export using MeshHandle = Core::IDHandle<MeshTag>;

export struct TextureTag {};
export using TextureHandle = Core::IDHandle<TextureTag>;