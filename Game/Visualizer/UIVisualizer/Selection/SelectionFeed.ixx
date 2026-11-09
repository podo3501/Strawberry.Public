export module UIVisualizer.SelectionFeed;

import std;
import UserInterface;
import UIVisualizer.SelectionHandoff;
import UIVisualizer.SelectionOverlay;
import UIVisualizer.WidgetSelection;
import Contract.Asset.Shared;
import Contract.Input.Service;
import Contract.Render.UI;
import Contract.Render.ITextMeasurer;
import Core.AssetPath;
import Core.Math;

// 고른 위젯을 화면에 보여 주고 Claude에게 알리는 부분 (구현: SelectionFeed.cpp)
// - 왼쪽 클릭은 위젯을 픽하고(WidgetSelection), C 키는 픽한 위젯을 선택한다 (이미 선택한 위젯이면 해제). Escape는 픽이 있으면 픽만 취소하고 없으면 선택을 모두 비운다.
//   픽은 체인의 위젯 이름을 적은 창 목록으로, 선택은 번호가 붙은 빨간 윤곽선으로 덧그린다.
// - 선택이 바뀔 때마다 선택 정보 파일(kFile)에 적는다 (형식은 SelectionReport.ixx).
// - 메시지를 보낼 때 훅이 그 파일을 읽어 가고 지우므로, 선택이 있는데 파일이 없어지면 넘어간 것으로 보고 선택을 비운다.
// - 레이아웃을 다시 읽으면 위젯이 새로 만들어지므로, 고른 위젯을 경로로 기억했다가(Paths) 같은 경로의 위젯을 다시 고른다(Restore). 없어진 위젯은 건너뛴다.
export class SelectionFeed
{
public:
    // storage 안의 이 파일에 선택 정보를 덮어쓴다. 저장소 루트의 .visualizer 폴더는 git에서 제외한다
    inline static const Core::AssetPath kFile = Core::AssetPath::Make(".visualizer/Selection.txt");

    // layerRoot는 위젯을 고르는 레이어의 루트이고, layoutLabel은 선택 정보 파일 머리에 적을 레이아웃 JSON의 이름이다 (저장소 루트 기준 전체 경로. Claude가 JSON을 찾는 데 쓴다).
    // layerRoot, storage, measurer(픽 창의 폭을 이름 글자 폭에 맞추는 데 쓴다)는 SelectionFeed보다 오래 살아야 한다
    SelectionFeed(Widget& layerRoot, IAssetStorage& storage, ITextMeasurer& measurer, std::string layoutLabel);

    void SetFont(FontHandle font) { m_overlay.SetFont(font); }       // 번호의 글꼴
    void SetBrush(BrushHandle brush) { m_overlay.SetBrush(brush); }  // 윤곽선으로 늘려 그릴 이미지

    // 지금 고른 위젯 목록을 선택 정보 파일에 덮어쓴다. 실패하면 로그를 남기고 false (선택 파일이 안 써져도 게임은 계속한다)
    bool Write();

    // 프레임마다 모드 중에 부른다. Escape(픽 취소, 없으면 선택 비움), 왼쪽 클릭(픽), 휠(픽을 체인을 따라 이동), C(픽한 위젯 선택/해제)를 처리한다.
    // 픽은 파일에 영향이 없으므로 선택이 바뀔 때만 선택 정보 파일을 다시 쓴다
    void HandleInput(const IInputService& input);

    // 프레임마다 모드 중에 부른다. 선택이 있는데 훅이 파일을 가져갔으면 다음 메시지가 1번부터 시작하도록 선택을 비우고 파일을 다시 쓴다
    void CheckHandoff(float dt);

    void Clear();   // 선택과 픽을 비우고, 선택이 있었으면 파일을 다시 쓴다 (선택이 이미 비어 있으면 파일을 다시 쓰지 않는다)

    // 고른 순서대로 (번호는 순서 + 1). 값을 바꾸는 쪽(미세 이동)도 쓰므로 const가 아니다
    std::span<Widget* const> Items() const { return m_selection.Items(); }

    // 선택을 먼저, 픽 창을 그 위(가장 위)에 덧그린다. screen은 픽 창이 화면 밖으로 나가지 않게 하는 데 쓴다
    void Draw(IOverlayView& overlay, const Core::Size& screen) const
    {
        m_overlay.Draw(overlay, m_selection.Items());
        m_overlay.DrawPick(overlay, screen, m_selection.PickWindow(), m_selection.Picked(), m_selection.PickAnchor());
    }

    // 레이아웃을 다시 읽는 동안의 선택 보존. 읽기 전에 Paths로 경로를 기억하고, 읽는 동안 Drop으로 비우고, 읽은 뒤 Restore로 되살린다
    std::vector<std::string> Paths() const;
    void Drop() { m_selection.Clear(); }   // 선택만 비운다 (파일은 곧 Restore가 쓰므로 쓰지 않는다)
    void Restore(std::span<const std::string> paths);   // 경로의 위젯을 찾아 다시 고르고 선택 정보 파일을 쓴다. 찾지 못한 경로는 건너뛴다

private:
    Widget& m_layerRoot;
    IAssetStorage& m_storage;
    std::string m_layoutLabel;
    WidgetSelection m_selection;
    SelectionHandoff m_handoff;
    SelectionOverlay m_overlay;
};
