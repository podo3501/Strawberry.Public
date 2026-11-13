export module DxRender.Resource:GridDebugMaterial;

import :DebugMaterial;
import DxRender.Definition;
import Contract.Render.Backend.Types;
import Core.TypeHierarchy;

export class GridDebugMaterialResource : public Core::TypeNode<GridDebugMaterialResource, DebugMaterialResource>
{
public:
    GridDebugMaterialResource() : 
        TypeNode{
            PipelineLibrary::Get(
                RegistryShader::Grid,
                RasterPreset::Default,
                PrimitiveTopologyType::Line)
        }
    {}

    bool IsReady() const noexcept override { return true; }
};