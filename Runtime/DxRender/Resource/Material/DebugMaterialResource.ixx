export module DxRender.Resource:DebugMaterial;

import DxRender.Definition;
import Contract.Render.IResource;

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