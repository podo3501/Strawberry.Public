export module UIVisualizer.UIVisualizer;

import std;
import UserInterface;
import UIVisualizer.LayoutLoader;
import UIVisualizer.SelectionOverlay;
import UIVisualizer.StatusOverlay;
import UIVisualizer.WidgetSelection;
import Contract.Asset.Shared;
import Contract.Input.Service;
import Contract.Render.UI;
import Core.AssetPath;
import Core.Math;

// 게임 화면에 UI JSON을 보여 주고 위젯을 골라 Claude에게 알리는 UIVisualizer 모드 전체 (구현: UIVisualizer.cpp)
// F1로 모드를 켜고 끈다. 켜져 있는 동안 입력은 UI 대신 위젯 선택으로 가고, 레이아웃 JSON이 바뀌면 자동으로 다시 읽는다.
// 읽기 오류 안내와 모드 표시는 모드와 상관없이 UI 위에 덧그린다. 고른 위젯은 윤곽선과 번호로 그리고, 선택이 바뀔 때마다 선택 정보 파일에 적는다.
export class UIVisualizer
{
public:
    // 번호와 안내 글자의 글꼴, 윤곽선으로 늘려 그릴 이미지. 이름은 resources가 아는 에셋 이름이다 (UIVisualizer이 자기 화면을 그리려고 쓴다)
    static constexpr std::string_view kFontName = "Graphics/Font/Pretendard-Regular.ttf";
    static constexpr std::string_view kOutlineBrushName = "Graphics/Texture/red.png";

    // storage 안의 이 파일에 선택 정보를 덮어쓴다 (형식은 SelectionReport.ixx). 저장소 루트의 .visualizer 폴더는 git에서 제외한다
    inline static const Core::AssetPath kSelectionFile = Core::AssetPath::Make(".visualizer/Selection.txt");

    // ui, storage, resources는 UIVisualizer보다 오래 살아야 한다.
    // layoutFile은 storage 안의 레이아웃 JSON이고, layoutLabel은 선택 정보 파일에 적을 그 파일의 이름이다 (저장소 루트 기준 전체 경로. Claude가 JSON을 찾는 데 쓴다)
    UIVisualizer(UserInterface& ui, IAssetStorage& storage, IWidgetResources& resources, Core::AssetPath layoutFile, std::string layoutLabel);

    UIVisualizer(const UIVisualizer&) = delete;
    UIVisualizer& operator=(const UIVisualizer&) = delete;

    // 글꼴과 윤곽선 브러시(kFontName, kOutlineBrushName)는 resources로 찾으므로 렌더 서비스를 초기화한 뒤 부른다.
    // 레이아웃 파일이 없으면 빈 레이아웃으로 첫 파일을 만들고, 읽고, 지난 실행에서 남은 선택 정보를 비운다.
    // 글꼴이나 브러시를 못 찾거나, 레이아웃 파일을 만들지 읽지 못하거나, 선택 정보 파일을 쓰지 못하면 false (모두 Error()로 로그를 남기고, 하나가 실패해도 나머지는 끝까지 한다).
    // 시작할 때의 일이므로 호출하는 쪽은 false이면 시작하지 않는다. 실행 중에 JSON을 다시 읽다가 실패하는 것은 이와 달리 이전 화면을 유지한다
    bool Initialize();

    // 레이아웃을 읽을 때마다(위젯이 새로 만들어진 뒤) 선택을 비우고 이 함수를 부른다. 콜백을 이름으로 다시 연결하는 데 쓴다
    void SetOnLoaded(std::function<void(UserInterface&)> onLoaded) { m_onLoaded = std::move(onLoaded); }

    // 프레임마다 입력을 처리한다 (F1, 모드 중에는 다시 읽기와 선택). 모드가 켜져 있으면 true이고, 이때 호출하는 쪽은 입력을 UI로 넘기지 않는다
    bool Update(float dt, const IInputService& input);

    // UI를 그린 다음 같은 오버레이에 덧그린다
    void Draw(IOverlayView& overlay, const Core::Size& screen) const;

    bool IsActive() const noexcept { return m_active; }

private:
    void SelectWidgets(const IInputService& input);
    void ClearSelection();    // 이미 비어 있으면 파일을 다시 쓰지 않는다
    bool WriteSelection();    // 지금 고른 위젯 목록을 선택 정보 파일에 덮어쓴다. 실패하면 로그를 남기고 false. 실행 중에는 결과를 쓰지 않는다 (선택 파일이 안 써져도 게임은 계속한다)

    UserInterface& m_ui;
    IAssetStorage& m_storage;
    IWidgetResources& m_resources;
    std::string m_layoutLabel;
    LayoutLoader m_layoutLoader;
    StatusOverlay m_statusOverlay;
    WidgetSelection m_selection;
    SelectionOverlay m_selectionOverlay;
    std::function<void(UserInterface&)> m_onLoaded;
    bool m_active{ false };
};
