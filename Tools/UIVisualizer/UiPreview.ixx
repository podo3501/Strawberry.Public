module;

#include <Windows.h>
#include <objbase.h>

export module UIVisualizer.UiPreview;

import std;
import UIVisualizer.PanelInput;
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
        const BrushHandle bg = resources.LoadBrush(BrushDesc{ Core::ResourceID::MakePath("Sprite/detailscreen_window_actions_bg.png") });

        m_ui->Resize(static_cast<float>(m_size.width), static_cast<float>(m_size.height));

        // 원본 이미지 512x141의 절반 크기. 호버는 밝게, 눌림은 더 밝게
        m_ui->Attach(std::make_unique<Button>(
            Core::Rect{ 40.f, 40.f, 256.f, 70.f }, ButtonBrushes::Tinted(bg, { 1.1f, 1.1f, 1.1f }, { 1.3f, 1.3f, 1.3f })), UILayer::Window);

        // 14x14 원본을 2배로 키워 보여 준다. 꺼짐/켜짐 각각 일반, 호버, 눌림 이미지를 쓴다
        auto sprite = [&](const char* name) { return resources.LoadBrush(BrushDesc{ Core::ResourceID::MakePath(std::format("Sprite/{}.png", name)) }); };
        ToggleSkin toggleSkin;
        toggleSkin.off = { sprite("toggle"), sprite("toggle hover"), sprite("toggle active") };
        toggleSkin.on = { sprite("toggle on"), sprite("toggle on hover"), sprite("toggle on active") };
        m_ui->Attach(std::make_unique<Toggle>(Core::Rect{ 40.f, 130.f, 28.f, 28.f }, toggleSkin), UILayer::Window);

        // 슬라이더와 진행 막대: 같은 틀(178x16)과 채움(154x8) 이미지를 9슬라이스로 늘려 쓴다.
        // 슬라이더를 움직이면 아래 진행 막대가 같은 비율로 따라간다
        GaugeSkin gauge;
        gauge.background = SlicedBrush::Sliced(sprite("build_menu_scrollbar_frame_horizontal"), { 8.f, 0.f, 8.f, 0.f }, { 178.f, 16.f });
        gauge.fill = SlicedBrush::Sliced(sprite("build_menu_scrollbar_inner_horizontal"), { 4.f, 0.f, 4.f, 0.f }, { 154.f, 8.f });
        gauge.fillInset = { 4.f, 4.f, 4.f, 4.f };

        SliderSkin sliderSkin;
        sliderSkin.bar = gauge;
        sliderSkin.thumb = sprite("slider thumb");
        sliderSkin.thumbSize = { 12.f, 12.f };
        auto& slider = m_ui->Attach(std::make_unique<Slider>(Core::Rect{ 40.f, 190.f, 200.f, 16.f }, sliderSkin, SliderAxis::Horizontal, 0.5f), UILayer::Window);
        auto& progress = m_ui->Attach(std::make_unique<ProgressBar>(Core::Rect{ 40.f, 230.f, 200.f, 16.f }, gauge, 0.5f), UILayer::Window);
        slider.SetOnChanged([&progress](float value) { progress.SetRatio(value); });

        // 글자 입력 칸: 평상시 textfield, 포커스를 받으면 textfield on 이미지 (9슬라이스)
        TextBoxSkin textSkin;
        constexpr SliceMargins textMargins{ 5.f, 5.f, 5.f, 5.f };   // 12x12 원본의 모서리 5픽셀은 늘어나지 않는다
        textSkin.normal = SlicedBrush::Sliced(sprite("textfield"), textMargins, { 12.f, 12.f });
        textSkin.focused = SlicedBrush::Sliced(sprite("textfield on"), textMargins, { 12.f, 12.f });
        textSkin.caret = resources.LoadBrush(BrushDesc{ Core::ResourceID::MakePath("Texture/Buttons/alpha_white.png") });
        textSkin.font = resources.LoadFont(FontDesc{ Core::ResourceID::MakePath("Font/Pretendard-Regular.ttf") });
        textSkin.fontSize = 18;
        textSkin.caretWidth = 2.f;
        m_ui->Attach(std::make_unique<TextBox>(
            m_ui->TextMeasurer(), Core::Rect{ 40.f, 270.f, 240.f, 32.f }, textSkin, "type here"), UILayer::Window);
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
