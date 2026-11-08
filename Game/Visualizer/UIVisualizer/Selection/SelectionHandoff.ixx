export module UIVisualizer.SelectionHandoff;

import std;
import Contract.Asset.Shared;
import Core.AssetPath;

// 선택 정보 파일이 Claude에게 넘어갔는지 알아본다.
// 메시지를 보낼 때 훅이 선택 정보를 읽어 가고 파일을 지우므로, 선택이 있는데 파일이 없어졌다면 넘어간 것이다.
// 이 클래스는 일정 간격으로 파일이 있는지만 확인하고, 선택을 비우는 일은 부른 쪽이 한다.
export class SelectionHandoff
{
public:
    static constexpr float kPollInterval = 0.1f;   // 파일이 있는지 확인하는 간격(초)

    // 프레임마다 부른다. kPollInterval이 지났을 때 file이 없으면 true (넘어갔다는 뜻)
    bool WasTaken(float dt, const IAssetStorage& storage, const Core::AssetPath& file)
    {
        m_sincePoll += dt;
        if (m_sincePoll < kPollInterval)
            return false;

        m_sincePoll = 0.f;
        return !storage.GetModifiedTime(file).has_value();
    }

private:
    float m_sincePoll{ 0.f };
};
