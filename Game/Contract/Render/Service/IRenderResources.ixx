export module Contract.Render.IRenderResources;

import Contract.Render.Handles;
import Contract.Render.Service.Descriptors;
import Contract.Render.Assets;

export struct IRenderResources
{
	virtual ~IRenderResources() = default;

	virtual FontHandle LoadFont(const FontDesc& desc) = 0;
	virtual bool ReleaseFont(FontHandle fh) = 0;

	virtual MeshHandle LoadMesh(const MeshDesc& desc, std::shared_ptr<MeshAsset> asset = nullptr) = 0;
	virtual bool ReleaseMesh(MeshHandle mh) = 0;

	virtual DebugMeshHandle LoadDebugMesh(const DebugMeshDesc& desc, std::shared_ptr<MeshAsset> asset = nullptr) = 0;

	virtual MaterialHandle LoadMaterial(const MaterialDesc& desc) = 0;
	virtual bool ReleaseMaterial(MaterialHandle mh) = 0;

	virtual DebugMaterialHandle LoadDebugMaterial(const DebugMaterialDesc& desc) = 0;
	virtual bool ReleaseDebugMaterial(DebugMaterialHandle dmh) = 0;

	virtual BrushHandle LoadBrush(const BrushDesc& desc) = 0;
	virtual bool ReleaseBrush(BrushHandle bh) = 0;

	virtual EnvironmentHandle LoadEnvironment(const EnvironmentDesc& desc) = 0;
	virtual bool ReleaseEnvironment(EnvironmentHandle eh) = 0;

	virtual void ReleaseAll() = 0;
};
