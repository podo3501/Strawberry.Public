export module Runtime.Render.Resource:GridDebugMaterial;

import :DebugMaterial;
import Runtime.Render.Definition;
import Client.Render.Definition;

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