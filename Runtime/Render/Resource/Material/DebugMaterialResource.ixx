export module Runtime.Render.Resource:DebugMaterial;

import Runtime.Render.Definition;
import Client.Render.IResource;

export class DebugMaterialResource : public IResource
{
public:
    virtual ~DebugMaterialResource() override = default;
    const PipelineState& GetPipelineState() const noexcept { return m_pipelineState; }

protected:
    explicit DebugMaterialResource(PipelineState pipelineState) :
        m_pipelineState{ std::move(pipelineState) }
    {}

private:
    const PipelineState m_pipelineState;
};