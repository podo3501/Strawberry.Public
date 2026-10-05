export module Runtime.Render.Resource:Material;

import std;
import Runtime.Render.Definition;
import Client.Render.IResource;
import Client.Render.Definition;
import Client.Asset.Data;

export class MaterialResource : public IResource
{
public:
    virtual ~MaterialResource() override = default;
    MaterialResource() = delete;

    MaterialType GetType() const noexcept { return m_type; }

    PipelineState GetPipelineState(
        const std::optional<RasterPreset>& rasterOverride,
        const std::optional<ShaderID>& shaderOverride) const
    {
        PipelineState result = m_pipelineState;

        if (rasterOverride)
            result.rasterState = RasterLibrary::Get(*rasterOverride);

        if (shaderOverride)
            result.shaderVariant.shaderID = *shaderOverride;

        return result;
    }

    const PipelineState& GetPipelineState() const noexcept { return m_pipelineState; }

protected:
    MaterialResource(MaterialType type, PipelineState pipelineState) :
        m_type{ type },
        m_pipelineState{ std::move(pipelineState) }
    {}

private:
    MaterialType m_type;
    PipelineState m_pipelineState;
};