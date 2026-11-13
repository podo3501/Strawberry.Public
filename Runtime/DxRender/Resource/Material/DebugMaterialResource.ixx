export module DxRender.Resource:DebugMaterial;

import DxRender.Definition;
import Contract.Render.IResource;
import Core.TypeHierarchy;

export class DebugMaterialResource : public Core::TypeNode<DebugMaterialResource, IResource>
{
public:
    ~DebugMaterialResource() override = default;
    const PipelineState& GetPipelineState() const noexcept { return m_pipelineState; }

protected:
    explicit DebugMaterialResource(PipelineState pipelineState) :
        m_pipelineState{ std::move(pipelineState) }
    {}

private:
    const PipelineState m_pipelineState;
};