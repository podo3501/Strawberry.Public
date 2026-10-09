export module Pipeline.Inspector:Renderers;

import std;
import :ImageRenderer;
import Core.Math;
import Pipeline.Renderer;
import DxRender.Core;
import DxRender.Shader;

export class InspectorRenderers
{
public:
    ~InspectorRenderers() = default;

    InspectorRenderers(Device& device, ShaderLibrary& shaderLibrary) :
        m_device{ device },
        m_pipelineCache{ device, shaderLibrary },
        m_imageRenderer{ m_pipelineCache }
    {}

    bool Initialize(const Core::Size& screenSize)
    {
        if (!m_imageRenderer.Initialize(m_device, screenSize)) return false;

        return true;
    }

    void SetScreenSize(const Core::Size& screenSize)
    {
        m_imageRenderer.SetScreenSize(screenSize);
    }

    InspectorImageRenderer& GetInspectorImageRenderer() noexcept
    {
        return m_imageRenderer;
    }

private:
    Device& m_device;
    PipelineCache m_pipelineCache;

    InspectorImageRenderer m_imageRenderer;
};