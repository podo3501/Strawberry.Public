export module Pipeline.Renderer:Renderers;

import :PipelineCache;
import :Config;
import :Shadow;
import :Surface;
import :DebugSurface;
import :UI;
import :Skybox;
import :Composite;
import DxRender.Core;

export class Renderers
{
public:
    ~Renderers() = default;
    Renderers(Device& device, ShaderLibrary& shaderLibrary) : 
        m_device{ device },
        m_pipelineCache{ device, shaderLibrary },
        m_shadowRenderer{ m_config.shadow, m_pipelineCache },
        m_surfRenderer{ m_config.surface, m_pipelineCache },
        m_debugSurfRenderer{ m_config.debug, m_pipelineCache },
        m_uiRenderer{ m_config.ui, m_pipelineCache },
        m_skyboxRenderer{ m_config.skybox, m_pipelineCache },
        m_compositeRenderer{ m_pipelineCache }
    {}

    bool Initialize()
    {
        if (!m_shadowRenderer.Initialize(m_device)) return false;
        if (!m_surfRenderer.Initialize(m_device)) return false;
        if (!m_debugSurfRenderer.Initialize(m_device)) return false;
        if (!m_uiRenderer.Initialize(m_device)) return false;
        if (!m_skyboxRenderer.Initialize(m_device)) return false;
        if (!m_compositeRenderer.Initialize(m_device)) return false;

        return true;
    }

    void ResetFrameResources(std::uint32_t slot)
    {
        m_shadowRenderer.ResetFrameResources(slot);
        m_surfRenderer.ResetFrameResources(slot);
        m_debugSurfRenderer.ResetFrameResources(slot);
        m_uiRenderer.ResetFrameResources(slot);
        m_skyboxRenderer.ResetFrameResources(slot);
    }

    ShadowRenderer& GetShadowRenderer() noexcept { return m_shadowRenderer; }
    SurfaceRenderer& GetSurfRenderer() noexcept { return m_surfRenderer; }
    DebugSurfaceRenderer& GetDebugSurfRenderer() noexcept { return m_debugSurfRenderer; }
    UIRenderer& GetUIRenderer() noexcept { return m_uiRenderer; }
    SkyboxRenderer& GetSkyboxRenderer() noexcept { return m_skyboxRenderer; }
    CompositeRenderer& GetCompositeRenderer() noexcept { return m_compositeRenderer; }

private:
    Device& m_device;
    RendererConfig m_config;
    PipelineCache m_pipelineCache;

    ShadowRenderer m_shadowRenderer;
    SurfaceRenderer m_surfRenderer;
    DebugSurfaceRenderer m_debugSurfRenderer;
    UIRenderer m_uiRenderer;
    SkyboxRenderer m_skyboxRenderer;
    CompositeRenderer m_compositeRenderer;
};