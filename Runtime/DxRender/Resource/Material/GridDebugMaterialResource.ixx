export module DxRender.Resource:GridDebugMaterial;

import :DebugMaterial;
import DxRender.Definition;
import Contract.Render.Definition;

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

    virtual bool IsReady() const noexcept override { return true; }
};