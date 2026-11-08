module;

#include <Windows.h>

module App.Application;

import std;
import Contract.Render.Shared;
import Contract.Input.Shared;
import Contract.Input.Backend;
import Core.Assert;
import Core.Logger;
import Core.TypeHierarchy;
import Core.Utils;

namespace
{
    constexpr Core::Size kInitialSize{ 1280, 720 };

    // 이 실행 파일 위치에서 거슬러 올라가 root.mark가 있는 저장소 루트를 찾는다
    std::filesystem::path RepositoryRoot()
    {
        wchar_t exePath[MAX_PATH]{};
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        return Core::FindRootByMarker(exePath, "root.mark");
    }

    // 리소스 폴더(Testing/Resource). 에셋, UI 레이아웃, UIVisualizer 파일이 모두 이 아래에 있다
    std::filesystem::path ResourceRoot()
    {
        return Core::ResolvePath(RepositoryRoot(), "Testing/Resource/");
    }

    // UI가 쓰는 키. 이 키들이 눌린 프레임에 한 번씩 UI로 넘긴다
    constexpr std::array kUiKeys{
        KeyCode::Tab, KeyCode::Escape, KeyCode::Enter, KeyCode::Space, KeyCode::Backspace, KeyCode::Delete,
        KeyCode::Home, KeyCode::End, KeyCode::Left, KeyCode::Right, KeyCode::Up, KeyCode::Down
    };

    // Window 레이어 바로 아래 자식 중 이름이 같은 위젯 (종류가 다르면 nullptr)
    template<typename T>
    T* FindInWindow(UserInterface& ui, std::string_view name)
    {
        for (const auto& child : WidgetInspector::Children(ui.Layer(UILayer::Window)))
            if (WidgetInspector::Name(*child) == name)
                return Core::Cast<T>(child.get());
        return nullptr;
    }

    // 서비스 팩토리가 null을 돌려주는 경우는 인자가 null일 때뿐이라 여기서는 일어나지 않는다. 일어나면 멤버 초기화에서 쓰러지기 전에 알린다
    template<typename T>
    std::unique_ptr<T> Required(std::unique_ptr<T> object)
    {
        Core::Assert(object != nullptr);
        return object;
    }
}

// 초기화 리스트의 순서는 Application.ixx의 멤버 선언 순서와 같다
Application::Application()
    : m_window{ L"Strawberry", kInitialSize }
    , m_resourceStorage{ CreateAssetStorage(ResourceRoot()) }
    , m_assetService{ Required(CreateAssetService(m_resourceStorage.get())) }
    , m_renderService{ Required(CreateRenderService(CreateRenderBackend(RenderConfig{}), m_assetService->GetAsyncLoader())) }
    , m_inputService{ Required(CreateInputService(CreateDxInputBackend(m_window.Handle()))) }
    , m_ui{ m_renderService->GetTextMeasurer() }
    , m_resources{ m_renderService->GetResources() }
    , m_visualizer{ m_ui, *m_resourceStorage, m_resources, kLayoutFile, "Testing/Resource/" + kLayoutFile.String() }
{
    m_visualizer.SetOnLoaded([this](UserInterface& ui) { LinkLayoutCallbacks(ui); });
}

bool Application::Initialize()
{
    if (!m_window.IsValid()) return false;

    m_size = m_window.ClientSize();
    if (!m_assetService->Initialize()) return false;
    if (!m_renderService->Initialize(m_window.Handle(), m_size)) return false;

    m_ui.Resize(static_cast<float>(m_size.width), static_cast<float>(m_size.height));
    // 파일이 없으면 빈 레이아웃으로 첫 JSON을 만든다. 글꼴이나 레이아웃을 읽지 못하는 등 하나라도 실패하면 시작하지 않는다 (원인은 Engine.log에 남는다)
    return m_visualizer.Initialize();
}

int Application::Run()
{
    using Clock = std::chrono::steady_clock;
    auto last = Clock::now();

    while (m_window.PumpMessages())
    {
        const auto now = Clock::now();
        const float dt = std::chrono::duration<float>(now - last).count();
        last = now;

        Frame(dt);
    }
    return 0;
}

void Application::Frame(float dt)
{
    const Core::Size size = m_window.ClientSize();
    if (size.width == 0 || size.height == 0)   // 최소화: 그리지 않고 쉰다
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return;
    }

    if (size != m_size)
    {
        m_size = size;
        m_renderService->Resize(size);
        m_ui.Resize(static_cast<float>(size.width), static_cast<float>(size.height));
    }

    m_inputService->Update(dt);

    if (!m_visualizer.Update(dt, *m_inputService))   // 모드 중이면 입력은 UI가 아니라 모드가 가져간다
    {
        m_ui.Update(m_inputService->GetMouseState());
        ForwardKeysToUi();
        if (const auto text = m_inputService->GetTextInput(); !text.empty())
            m_ui.OnTextInput(text);
    }
    m_ui.Tick(dt);

    OverlayViewContext viewCtx{ kOverlayView };
    viewCtx.target.camera = m_uiCamera.BuildData(m_size, viewCtx.target.viewport);
    auto& overlay = m_renderService->GetFrameRecorder().AcquireView(viewCtx);
    m_ui.Draw(overlay);
    m_visualizer.Draw(overlay, m_size);   // UI 위에 덧그린다

    m_renderService->Update();
    m_renderService->Render();
}

void Application::ForwardKeysToUi()
{
    const bool shift = m_inputService->GetKey(KeyCode::LeftShift).IsDown() || m_inputService->GetKey(KeyCode::RightShift).IsDown();
    for (const KeyCode key : kUiKeys)
    {
        const ButtonState& state = m_inputService->GetKey(key);
        if (state.IsPressed() || state.IsRepeated())   // 누른 프레임과, 계속 누르고 있을 때의 반복 시점
            m_ui.OnKeyDown(key, shift);
    }
}

// 콜백은 JSON에 저장되지 않으므로 불러온 뒤 이름으로 위젯을 찾아 다시 연다 (샘플 레이아웃용. 게임 UI가 생기면 그 UI에 맞게 바꾼다)
void Application::LinkLayoutCallbacks(UserInterface& ui)
{
    auto* slider = FindInWindow<Slider>(ui, "Slider");
    auto* progress = FindInWindow<ProgressBar>(ui, "Progress");
    if (slider && progress)
        slider->SetOnChanged([progress](float value) { progress->SetRatio(value); });   // 슬라이더를 움직이면 진행 막대가 같은 비율로 따라간다
}
