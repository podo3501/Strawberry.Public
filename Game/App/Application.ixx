export module App.Application;

import std;
import App.GameWindow;
import UserInterface;
import UIVisualizer.UiResources;
import UIVisualizer.UIVisualizer;
import Graphics.SceneObject;
import Contract.Render.Shared;
import Contract.Render.Service;
import Contract.Input.Service;
import Contract.Asset.Service;
import Core.AssetPath;
import Core.Math;

// 게임을 실행하는 얇은 진입 층. 창과 서비스를 조립하고 프레임 루프를 돈다 (구현: Application.cpp)
// 로거와 COM은 Application을 만들기 전에 초기화되어 있어야 하고, COM은 Application이 해제된 뒤에 정리해야 한다
export class Application
{
public:
    Application();

    // 서비스를 초기화한다. 실패하면 false
    bool Initialize();

    // 창이 닫힐 때까지 프레임 루프를 돈다. 종료 코드를 돌려준다
    int Run();

private:
    static constexpr ViewID kOverlayView = 1;
    inline static const Core::AssetPath kLayoutFile = Core::AssetPath::Make("UI/Sample/Window.json");   // Testing/Resource 아래

    void Frame(float dt);
    void ForwardKeysToUi();
    void LinkLayoutCallbacks(UserInterface& ui);

    // 선언 순서가 곧 만드는 순서이자 해제의 역순이다.
    // 창은 입력 백엔드가 서브클래싱하므로 가장 먼저 만들고 가장 나중에 해제한다
    GameWindow m_window;
    std::unique_ptr<IAssetStorage> m_resourceStorage;   // Testing/Resource 전체. 에셋(Graphics/...), UI 레이아웃(UI/...), UIVisualizer이 Claude에게 남기는 파일(.visualizer/...)이 모두 이 아래에 있다
    std::unique_ptr<IAssetService> m_assetService;
    std::unique_ptr<IRenderService> m_renderService;
    std::unique_ptr<IInputService> m_inputService;
    UserInterface m_ui;
    UiResources m_resources;
    UIVisualizer m_visualizer;   // F1로 켜는 UIVisualizer 모드. 켜져 있는 동안 입력은 UI 대신 모드가 가져간다

    bool m_visualizerWasActive{ false };   // 모드가 켜진 프레임을 알아내려고 지난 프레임의 상태를 기억한다
    Core::Size m_size{};
    UICamera m_uiCamera;
};
