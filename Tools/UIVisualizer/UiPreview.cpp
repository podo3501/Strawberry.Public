module;

#include <Windows.h>
#undef DrawText   // winuser.h의 DrawText 매크로가 IOverlayView::DrawText 호출을 DrawTextW로 바꾸지 않게 한다

module UIVisualizer.UiPreview;

import std;
import UserInterface;
import Contract.Render.Shared;
import Contract.Render.Service;
import Contract.Asset.Service;
import Contract.Input.Shared;
import Core.Assert;
import Core.TypeHierarchy;
import Core.Utils;
import Core.Logger;

namespace
{
    // 이 실행 파일 위치에서 거슬러 올라가 root.mark가 있는 저장소 루트를 찾는다
    std::filesystem::path RepositoryRoot()
    {
        wchar_t exePath[MAX_PATH]{};
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        return Core::FindRootByMarker(exePath, "root.mark");
    }

    // 서비스 팩토리가 null을 돌려주는 경우는 인자가 null일 때뿐이라 여기서는 일어나지 않는다. 일어나면 멤버 초기화에서 쓰러지기 전에 알린다
    template<typename T>
    std::unique_ptr<T> Required(std::unique_ptr<T> object)
    {
        Core::Assert(object != nullptr);
        return object;
    }
}

// 초기화 리스트의 순서는 UiPreview.ixx의 멤버 선언 순서와 같다
UiPreview::UiPreview()
    : m_layoutDir{ Core::ResolvePath(RepositoryRoot(), "Testing/Resource/UI/") }
    , m_assetStorage{ CreateAssetStorage(Core::ResolvePath(RepositoryRoot(), "Testing/Resource/Graphics/")) }
    , m_layoutStorage{ CreateAssetStorage(m_layoutDir) }
    , m_assetService{ Required(CreateAssetService(m_assetStorage.get())) }
    , m_renderService{ Required(CreateRenderService(CreateRenderBackend(RenderConfig{}), m_assetService->GetAsyncLoader())) }
    , m_resources{ m_renderService->GetResources() }
    , m_ui{ m_renderService->GetTextMeasurer() }
{}

bool UiPreview::Initialize(const Core::Size& size)
{
    m_size = size;

    std::error_code directoryError;
    std::filesystem::create_directories(m_layoutDir / "Sample", directoryError);

    if (!m_assetService->Initialize()) return false;
    if (!m_renderService->Initialize(nullptr, size)) return false; // hwnd 없음: 오프스크린

    m_statusFont = m_resources.Font("Font/Pretendard-Regular.ttf");
    m_ui.Resize(static_cast<float>(m_size.width), static_cast<float>(m_size.height));
    LoadOrSeedSampleUi();
    m_layoutWatcher.Watch(m_layoutDir / kWindowFile.ToPath());   // 지금 읽은(또는 만든) 파일은 처리한 것으로 보고 이후의 변경만 본다
    return true;
}

void UiPreview::Resize(const Core::Size& size)
{
    if (size.width == 0 || size.height == 0 || size == m_size) return;

    m_size = size;
    m_renderService->Resize(size);
    m_ui.Resize(static_cast<float>(size.width), static_cast<float>(size.height));
}

void UiPreview::ApplyInput(const PanelInput& input)
{
    m_ui.Update(input.mouse);
    for (const KeyCode key : input.keys)
        m_ui.OnKeyDown(key, input.shift);
    if (!input.text.empty())
        m_ui.OnTextInput(input.text);
}

bool UiPreview::Frame(float dt, FrameImage& out)
{
    if (m_layoutWatcher.Poll(dt))
        ReloadLayout();

    auto& frameRecorder = m_renderService->GetFrameRecorder();

    OverlayViewContext viewCtx{ kOverlayView };
    viewCtx.target.camera = m_uiCamera.BuildData(m_size, viewCtx.target.viewport);
    auto& overlay = frameRecorder.AcquireView(viewCtx);

    m_ui.Tick(dt);
    m_ui.Draw(overlay);
    DrawLayoutError(overlay);   // UI 위에 덧그린다

    m_renderService->Update();
    m_renderService->Render();
    return m_renderService->ReadbackFrame(out);
}

// JSON이 기준이다. 파일이 있으면 불러오기만 한다. 파일이 없을 때만 코드 샘플로 첫 JSON을 만든다.
// 파일이 있는데 읽지 못하면(깨짐, 모르는 위젯 종류, 이름 겹침 등) 덮어쓰지 않고 오류만 남긴다. 고치던 내용을 잃지 않기 위해서다
void UiPreview::LoadOrSeedSampleUi()
{
    std::string existing;
    if (!m_layoutStorage->ReadText(kWindowFile, existing))
    {
        BuildSampleUi();
        if (!m_ui.SaveLayer(UILayer::Window, *m_layoutStorage, m_resources, kWindowFile))
            Core::Logger::Error("UI 레이아웃 저장 실패: {}", kWindowFile.String());
    }
    else if (!m_ui.LoadLayer(UILayer::Window, *m_layoutStorage, m_resources, kWindowFile))
    {
        ReportLoadFailure();
        return;
    }

    LinkSampleCallbacks();
}

// 파일이 바뀌어 안정되면 다시 읽는다. 읽지 못하면 화면은 마지막으로 성공한 상태를 그대로 두고(LoadLayer가 레이어를 건드리지 않는다) 안내만 띄운다.
// 파일을 고치면 수정 시각이 바뀌어 다시 시도한다. 위젯이 새로 만들어지므로 슬라이더 값 같은 화면에서 조작한 상태는 JSON에 적힌 값으로 돌아간다
void UiPreview::ReloadLayout()
{
    if (!m_ui.LoadLayer(UILayer::Window, *m_layoutStorage, m_resources, kWindowFile))
    {
        ReportLoadFailure();
        return;
    }

    m_layoutError.clear();
    LinkSampleCallbacks();
    Core::Logger::Info("UI 레이아웃을 다시 읽음: {}", kWindowFile.String());
}

void UiPreview::ReportLoadFailure()
{
    Core::Logger::Error("UI 레이아웃 불러오기 실패: {}", kWindowFile.String());
    m_layoutError = std::format("{} 읽기 실패 (Engine.log 참고). 마지막으로 읽은 화면을 보여 주는 중", kWindowFile.String());
}

// 읽기에 실패했을 때만 화면 왼쪽 위에 이유를 그린다. DxRender 글자라서 미리보기 텍스처 안에 들어간다
void UiPreview::DrawLayoutError(IOverlayView& overlay) const
{
    if (m_layoutError.empty() || !m_statusFont)
        return;

    constexpr float kMargin = 8.f;
    const Core::Rect bounds{ kMargin, kMargin, static_cast<float>(m_size.width) - kMargin * 2.f, static_cast<float>(m_size.height) - kMargin * 2.f };
    TextStyle style;
    style.color = Core::Color{ 1.f, 0.35f, 0.35f, 1.f };
    overlay.DrawText(m_statusFont, TextRenderMode::MTSDF, m_layoutError, kStatusFontSize, bounds, TextLayout{}, style);
}

template<typename T>
T* UiPreview::FindInWindow(std::string_view name)
{
    for (const auto& child : WidgetInspector::Children(m_ui.Layer(UILayer::Window)))
        if (WidgetInspector::Name(*child) == name)
            return Core::Cast<T>(child.get());
    return nullptr;
}

// 콜백은 JSON에 저장되지 않으므로 불러온 뒤 이름으로 위젯을 찾아 다시 연다
void UiPreview::LinkSampleCallbacks()
{
    auto* slider = FindInWindow<Slider>("Slider");
    auto* progress = FindInWindow<ProgressBar>("Progress");
    if (slider && progress)
        slider->SetOnChanged([progress](float value) { progress->SetRatio(value); });   // 슬라이더를 움직이면 진행 막대가 같은 비율로 따라간다
}

void UiPreview::BuildSampleUi()
{
    const BrushHandle bg = m_resources.Brush("Sprite/detailscreen_window_actions_bg.png");

    // 원본 이미지 512x141의 절반 크기. 호버는 밝게, 눌림은 더 밝게
    auto& button = m_ui.Attach(std::make_unique<Button>(
        Core::Rect{ 40.f, 40.f, 256.f, 70.f }, ButtonBrushes::Tinted(bg, { 1.1f, 1.1f, 1.1f }, { 1.3f, 1.3f, 1.3f })), UILayer::Window);
    button.SetName("Button");

    // 14x14 원본을 2배로 키워 보여 준다. 꺼짐/켜짐 각각 일반, 호버, 눌림 이미지를 쓴다
    auto sprite = [&](const char* name) { return m_resources.Brush(std::format("Sprite/{}.png", name)); };
    ToggleSkin toggleSkin;
    toggleSkin.off = { sprite("toggle"), sprite("toggle hover"), sprite("toggle active") };
    toggleSkin.on = { sprite("toggle on"), sprite("toggle on hover"), sprite("toggle on active") };
    m_ui.Attach(std::make_unique<Toggle>(Core::Rect{ 40.f, 130.f, 28.f, 28.f }, toggleSkin), UILayer::Window).SetName("Toggle");

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
    m_ui.Attach(std::make_unique<Slider>(Core::Rect{ 40.f, 190.f, 200.f, 16.f }, sliderSkin, SliderAxis::Horizontal, 0.5f), UILayer::Window).SetName("Slider");
    m_ui.Attach(std::make_unique<ProgressBar>(Core::Rect{ 40.f, 230.f, 200.f, 16.f }, gauge, 0.5f), UILayer::Window).SetName("Progress");

    // 글자 입력 칸: 평상시 textfield, 포커스를 받으면 textfield on 이미지 (9슬라이스)
    TextBoxSkin textSkin;
    constexpr SliceMargins textMargins{ 5.f, 5.f, 5.f, 5.f };   // 12x12 원본의 모서리 5픽셀은 늘어나지 않는다
    textSkin.normal = SlicedBrush::Sliced(sprite("textfield"), textMargins, { 12.f, 12.f });
    textSkin.focused = SlicedBrush::Sliced(sprite("textfield on"), textMargins, { 12.f, 12.f });
    textSkin.caret = m_resources.Brush("Texture/Buttons/alpha_white.png");
    textSkin.font = m_resources.Font("Font/Pretendard-Regular.ttf");
    textSkin.fontSize = 18;
    textSkin.caretWidth = 2.f;
    m_ui.Attach(std::make_unique<TextBox>(
        m_ui.TextMeasurer(), Core::Rect{ 40.f, 270.f, 240.f, 32.f }, textSkin, "type here"), UILayer::Window).SetName("TextBox");
}
