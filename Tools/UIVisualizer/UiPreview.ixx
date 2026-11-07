export module UIVisualizer.UiPreview;

import std;
import UIVisualizer.FileChangeDetector;
import UIVisualizer.PanelInput;
import UIVisualizer.UiResources;
import UserInterface;
import Graphics.SceneObject;
import Contract.Render.Shared;
import Contract.Render.Service;
import Contract.Render.UI;
import Contract.Asset.Service;
import Core.AssetPath;
import Core.Math;

// 엔진 UI를 창 없이 텍스처에 그려서 CPU로 돌려주는 호스트. 에셋/렌더 서비스와 UserInterface를 소유한다.
// UI는 Testing/Resource/UI/Sample/Window.json에서 불러오고, 파일이 바뀌면 다시 읽는다 (구현: UiPreview.cpp)
export class UiPreview
{
public:
    // 서비스를 만든다 (실패할 수 있는 초기화는 Initialize에서 한다). 렌더 백엔드가 D3D 장치를 만들므로
    // 로거와 COM은 UiPreview를 만들기 전에 초기화되어 있어야 하고, COM은 UiPreview가 해제된 뒤에 정리해야 한다
    UiPreview();

    // 서비스를 초기화하고 UI를 불러온다. 실패하면 false
    bool Initialize(const Core::Size& size);
    void Resize(const Core::Size& size);

    UserInterface& Ui() { return m_ui; }

    // 패널에서 모은 입력을 UI에 넘긴다. Frame보다 먼저 부른다
    void ApplyInput(const PanelInput& input);

    // UI를 한 프레임 그리고 결과를 out에 읽는다. 읽기에 성공하면 true
    bool Frame(float dt, FrameImage& out);

private:
    static constexpr ViewID kOverlayView = 1;
    static constexpr std::uint32_t kStatusFontSize = 16;
    inline static const Core::AssetPath kWindowFile = Core::AssetPath::Make("Sample/Window.json");   // Testing/Resource/UI 아래. 레이어마다 파일 하나

    void LoadOrSeedSampleUi();
    void ReloadLayout();
    void ReportLoadFailure();
    void LinkSampleCallbacks();
    void BuildSampleUi();
    void DrawLayoutError(IOverlayView& overlay) const;

    // Window 레이어 바로 아래 자식 중 이름이 같은 위젯 (종류가 다르면 nullptr). UiPreview.cpp 안에서만 쓰는 템플릿이라 정의도 거기에 있다
    template<typename T>
    T* FindInWindow(std::string_view name);

    // 선언 순서가 곧 만드는 순서이자(생성자의 초기화 리스트와 같다) 해제의 역순이다.
    // 아래쪽이 위쪽을 참조하므로 UserInterface -> UiResources -> 렌더 서비스 -> 에셋 서비스 순으로 해제된다
    std::filesystem::path m_layoutDir;   // Testing/Resource/UI. 에셋 스캔 대상인 Graphics 폴더와 따로 둔다
    std::unique_ptr<IAssetStorage> m_assetStorage;
    std::unique_ptr<IAssetStorage> m_layoutStorage;   // UI 레이아웃 JSON 전용
    std::unique_ptr<IAssetService> m_assetService;
    std::unique_ptr<IRenderService> m_renderService;
    UiResources m_resources;
    UserInterface m_ui;

    Core::Size m_size{};
    UICamera m_uiCamera;
    FileChangeDetector m_layoutWatcher;   // 레이아웃 JSON이 바뀌었는지 본다
    std::string m_layoutError;            // 레이아웃을 읽지 못했을 때의 안내 문구. 문제가 없으면 비어 있다
    FontHandle m_statusFont;              // 안내 문구를 그리는 폰트
};
