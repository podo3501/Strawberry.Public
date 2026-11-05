module;

#include <Windows.h>
#include <objbase.h>

export module UITool.UiPreview;

import std;
import UITool.PanelInput;
import UserInterface;
import Graphics.SceneObject;
import Contract.Render.Shared;
import Contract.Render.Service;
import Contract.Asset.Service;
import Contract.Input.Shared;
import Core.Logger;
import Core.Math;
import Core.Utils;

// 엔진 UI를 창 없이 텍스처에 그려서 CPU로 돌려주는 호스트. 에셋/렌더 서비스와 UserInterface를 소유한다.
export class UiPreview
{
public:
    ~UiPreview()
    {
        m_ui.reset();            // 렌더 서비스의 측정기를 쓰므로 먼저 해제한다
        m_renderService.reset(); // CoUninitialize보다 먼저 해제해야 한다
        if (m_comInitialized)
            CoUninitialize();
    }

    bool Initialize(const Core::Size& size)
    {
        Core::Logger::Initialize();
        m_comInitialized = SUCCEEDED(CoInitialize(nullptr));
        m_size = size;

        wchar_t exePath[MAX_PATH]{};
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        const auto rootPath = Core::FindRootByMarker(exePath, "root.mark");
        const auto resPath = Core::ResolvePath(rootPath, "Testing/Resource/Graphics/");

        m_assetStorage = CreateAssetStorage(resPath);
        m_assetService = CreateAssetService(m_assetStorage.get());
        m_metaRegistry = CreateAssetMetaRegistry(m_assetService->GetAsyncLoader());
        m_assetService->Initialize(m_metaRegistry.get());
        m_metaRegistry->Initialize();
        if (!m_metaRegistry->Scan(resPath)) return false;

        m_renderService = CreateRenderService(CreateRenderBackend(RenderConfig{}), m_assetService->GetAsyncLoader());
        if (!m_renderService) return false;
        if (!m_renderService->Initialize(nullptr, size)) return false; // hwnd 없음: 오프스크린

        m_ui = std::make_unique<UserInterface>(m_renderService->GetTextMeasurer());
        BuildSampleUi();
        return true;
    }

    void Resize(const Core::Size& size)
    {
        if (size.width == 0 || size.height == 0 || size == m_size) return;

        m_size = size;
        m_renderService->Resize(size);
        m_ui->Resize(static_cast<float>(size.width), static_cast<float>(size.height));
    }

    UserInterface& Ui() { return *m_ui; }

    // 패널에서 모은 입력을 UI에 넘긴다. Frame보다 먼저 부른다
    void ApplyInput(const PanelInput& input)
    {
        m_ui->Update(input.mouse);
        for (const KeyCode key : input.keys)
            m_ui->OnKeyDown(key, input.shift);
        if (!input.text.empty())
            m_ui->OnTextInput(input.text);
    }

    // UI를 한 프레임 그리고 결과를 out에 읽는다. 읽기에 성공하면 true
    bool Frame(float dt, FrameImage& out)
    {
        auto& frameRecorder = m_renderService->GetFrameRecorder();

        OverlayViewContext viewCtx{ kOverlayView };
        viewCtx.target.camera = m_uiCamera.BuildData(m_size, viewCtx.target.viewport);
        auto& overlay = frameRecorder.AcquireView(viewCtx);

        m_ui->Tick(dt);
        m_ui->Draw(overlay);

        m_renderService->Update();
        m_renderService->Render();
        return m_renderService->ReadbackFrame(out);
    }

private:
    static constexpr ViewID kOverlayView = 1;

    void BuildSampleUi()
    {
        auto& resources = m_renderService->GetResources();
        auto brush = [&](const char* path) { return resources.LoadBrush(BrushDesc{ Core::ResourceID::MakePath(path) }); };
        const BrushHandle blue = brush("Texture/Buttons/BlueButton.png");
        const BrushHandle red = brush("Texture/red.png");
        const BrushHandle white = brush("Texture/Buttons/alpha_white.png");
        const FontHandle font = resources.LoadFont(FontDesc{ Core::ResourceID::MakePath("Font/Pretendard-Regular.ttf") });

        m_ui->Resize(static_cast<float>(m_size.width), static_cast<float>(m_size.height));

        auto& label = m_ui->Attach(std::make_unique<Text>(
            m_ui->TextMeasurer(), font, TextRenderMode::MTSDF, "Strawberry UI",
            20, Core::Rect{ 40.f, 120.f, 300.f, 30.f }), UILayer::Window);

        auto& button = m_ui->Attach(std::make_unique<Button>(
            Core::Rect{ 40.f, 40.f, 200.f, 60.f }, ButtonBrushes{ blue, blue, red }), UILayer::Window);
        button.SetOnClick([&label, count = 0]() mutable { label.SetText(std::format("Clicked {}", ++count)); });

        TextBoxSkin skin;
        skin.normal = white;
        skin.focused = blue;
        skin.caret = red;
        skin.font = font;
        skin.fontSize = 18;
        skin.caretWidth = 2.f;
        m_ui->Attach(std::make_unique<TextBox>(
            m_ui->TextMeasurer(), Core::Rect{ 40.f, 170.f, 300.f, 36.f }, skin, "type here"), UILayer::Window);
    }

    bool m_comInitialized{ false };
    Core::Size m_size{};
    std::unique_ptr<IAssetStorage> m_assetStorage;
    std::unique_ptr<IAssetService> m_assetService;
    std::unique_ptr<IAssetMetaRegistry> m_metaRegistry;
    std::unique_ptr<IRenderService> m_renderService;
    UICamera m_uiCamera;
    std::unique_ptr<UserInterface> m_ui;   // 렌더 서비스의 측정기가 필요해서 서비스를 만든 뒤에 만든다
};
