module;

#include <wrl.h>
#include <DirectXMath.h>
#include "d3dx12.h"

export module Pipeline.Inspector:ImageRenderer;

import std;
import Core.Math;
import Core.Utils;
import Pipeline.Renderer;
import Runtime.Render.Core;
import Runtime.Render.Command;
import Runtime.Render.Allocator;
import Runtime.Render.Helper;
import Runtime.Render.Definition;
import Client.Render.Definition;

export struct InspectorTextureCB
{
    std::uint32_t srvIndex;
};

export struct InspectorDrawCB
{
    DirectX::XMFLOAT4X4 world;
    DirectX::XMFLOAT4X4 projection;

    DirectX::XMFLOAT2 imageSize;
    DirectX::XMFLOAT2 padding{};
};

export class InspectorImageRenderer
{
public:
    ~InspectorImageRenderer() = default;
    InspectorImageRenderer() = delete;
    explicit InspectorImageRenderer(PipelineCache& pipelineCache) noexcept
        : m_pipelineCache{ pipelineCache }
    {
        m_pipelineState = PipelineLibrary::Get(RegistryShader::InspectorImage, RasterPreset::NoCull);
    }

    bool Initialize(Device& device, const Core::Size& screenSize)
    {
        m_drawCBAllocator.Initialize<InspectorDrawCB>(device, MaxImage);

        if (!CreateRootSignature(device)) return false;
        if (!CreateDefaultPSOs()) return false;
        SetScreenSize(screenSize);

        return true;
    }

    void PrepareFrame(std::uint32_t slot)
    {
        m_drawCBAllocator.Reset(slot);
    }

    void BeginFrame(CommandList& cmd)
    {
        cmd.SetGraphicsRootSignature(m_rootSignature.Get());
    }

    void BindPipeline(CommandList& cmd)
    {
        cmd.SetPipelineState(GetPipeline(m_pipelineState), m_pipelineState.topologyType);
    }

    void Draw(CommandList& cmd, UINT srvIndex)
    {
        InspectorTextureCB indices{};
        indices.srvIndex = srvIndex;
        auto cb = UploadDrawCB();

        cmd->SetGraphicsRoot32BitConstants(Core::ToIndex(RootSlot::ResourceIndices), 1, &indices, 0);
        cmd->SetGraphicsRootConstantBufferView(Core::ToIndex(RootSlot::DrawCB), cb);

        cmd->DrawInstanced(6, 1, 0, 0);
    }

    void SetScreenSize(const Core::Size& size)
    {
        m_screenSize = size;

        m_projection = Core::Matrix::OrthographicOffCenter(
            0.0f,
            static_cast<float>(size.width),
            static_cast<float>(size.height),
            0.0f,
            0.0f,
            1.0f
        );
    }

private:
    static constexpr UINT MaxImage = 10;
    static constexpr float ImageSize = 256.0f;
    static constexpr float Margin = 16.0f; // 전체 창에서 여백

    enum class RootSlot : std::uint32_t
    {
        ResourceIndices = 0,
        DrawCB = 1
    };

    bool CreateRootSignature(Device& device)
    {
        RootSignatureBuilder builder;

        builder.Add32BitConstants(Core::ToIndex(RootSlot::ResourceIndices), 1);
        builder.AddCBV(Core::ToIndex(RootSlot::DrawCB));
        builder.AddPointSampler(0);

        builder.AddFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        m_rootSignature = builder.Build(device);
        return m_rootSignature != nullptr;
    }

    bool CreateDefaultPSOs()
    {
        if (CreatePSO(m_pipelineState) == nullptr) return false;

        return true;
    }

    ID3D12PipelineState* CreatePSO(const PipelineState& pipelineState)
    {
        return m_pipelineCache.GetOrCreate(
            pipelineState,
            m_rootSignature.Get(),
            [&](D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso)
            {
                pso.NumRenderTargets = 1;
                pso.RTVFormats[0] = RenderFormat::BackBufferSRGBView;

                pso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
                pso.DepthStencilState.DepthEnable = FALSE;
                pso.DepthStencilState.StencilEnable = FALSE;

                auto& rtBlend = pso.BlendState.RenderTarget[0];

                rtBlend.BlendEnable = FALSE;
                rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
            });
    }

    ID3D12PipelineState* GetPipeline(const PipelineState& pipelineState)
    {
        auto* pipeline = m_pipelineCache.Find(pipelineState);
        if (pipeline)
            return pipeline;

        return CreatePSO(pipelineState);
    }

    D3D12_GPU_VIRTUAL_ADDRESS UploadDrawCB()
    {
        InspectorDrawCB drawCB{};

        Core::Matrix world =
            Core::Matrix::Scale(ImageSize, ImageSize, 1.0f) *
            Core::Matrix::Translation(
                m_screenSize.width - ImageSize - Margin,
                m_screenSize.height - ImageSize - Margin,
                0.0f);

        DirectX::XMMATRIX xmWorld = ToDXMatrix(world);
        XMStoreFloat4x4(&drawCB.world, DirectX::XMMatrixTranspose(xmWorld));

        DirectX::XMMATRIX xmProj = ToDXMatrix(m_projection);
        XMStoreFloat4x4(&drawCB.projection, DirectX::XMMatrixTranspose(xmProj));

        drawCB.imageSize = { ImageSize, ImageSize };

        return m_drawCBAllocator.AllocateConstant(drawCB);
    }

    PipelineState m_pipelineState;
    PipelineCache& m_pipelineCache;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

    Core::Size m_screenSize{};
    FrameConstantAllocator m_drawCBAllocator;
    Core::Matrix m_projection;
};