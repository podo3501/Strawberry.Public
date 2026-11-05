export module UserInterface.Widget:WidgetSerializer;

import std;
import :Widget;
import :WidgetResources;
import Contract.Asset.Shared;
import Contract.Render.ITextMeasurer;

// 위젯 트리를 JSON 텍스트로 IAssetStorage에 저장하고 읽어 온다. 파일이 어디에 어떻게 놓이는지는 storage가 안다.
// 파일 모양: { "version": 1, "root": { "type": "Group", ...공통 속성..., "children": [ ... ] } }
// - 저장: 트리 안에 저장을 지원하지 않는 위젯(WidgetSerializer.cpp의 표에 없는 클래스)이 있거나 리소스 이름을 모르는 핸들이 있으면 아무것도 쓰지 않고 false.
//   표에는 정확한 클래스를 등록하므로, 등록하지 않은 파생 클래스는 부모 이름으로 저장되지 않고 저장이 거부된다.
// - 불러오기: 파일이 없거나, 형식이 깨졌거나, 지원하지 않는 버전이거나, 모르는 위젯 종류나 리소스 이름이 있으면 nullptr.
//   (모르는 종류를 건너뛰면 다시 저장할 때 내용이 사라지므로 통째로 실패로 본다)
// 위젯 종류를 늘릴 때는 그 위젯에 Serialize를 구현하고 WidgetSerializer.cpp의 표에 한 줄(파일에 적는 이름, 클래스, 생성 함수) 더한다.
// 파일에 적는 이름은 한 번 정하면 바꾸지 않는다 (옛 파일을 읽을 수 없게 된다).
// 위젯을 불러올 때 필요한 외부 서비스. 저장 파일에는 없고 실행 중에만 있는 참조라서 Load에 넘긴다 (저장할 때는 필요 없다).
// 둘 다 불러온 UI보다 오래 살아야 한다.
export struct WidgetLoadContext
{
    ITextMeasurer& measurer;   // 글자를 그리는 위젯(Text, TextBox, ...)이 쓴다
    Widget& popupHost;         // 목록을 펼치는 위젯(Dropdown)이 쓰는 팝업 레이어. 화면 전체를 덮고 원점이 화면 (0,0)이어야 한다 (UserInterface의 Popup 레이어)
};

export class WidgetSerializer
{
public:
    static constexpr int kVersion = 1;

    static bool Save(Widget& root, IAssetStorage& storage, IWidgetResources& resources, const std::filesystem::path& filename);
    static std::unique_ptr<Widget> Load(IAssetStorage& storage, IWidgetResources& resources, const WidgetLoadContext& context, const std::filesystem::path& filename);

private:
    struct Impl;   // 위젯의 닫힌 멤버(Serialize, 자식)에 접근하는 구현. WidgetSerializer의 friend 권한을 이어받는다
};
