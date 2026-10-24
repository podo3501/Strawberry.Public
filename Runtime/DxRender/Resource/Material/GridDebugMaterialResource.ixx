export module DxRender.Resource:GridDebugMaterial;

import :DebugMaterial;
import DxRender.Definition;
import Contract.Render.Backend.Types;

export class GridDebugMaterialResource : public DebugMaterialResource
{
public:
    GridDebugMaterialResource() : 
        DebugMaterialResource{
            PipelineLibrary::Get(
                RegistryShader::Grid,
                RasterPreset::Default,
                PrimitiveTopologyType::Line)
        }
    {}

    bool IsReady() const noexcept override { return true; }
};