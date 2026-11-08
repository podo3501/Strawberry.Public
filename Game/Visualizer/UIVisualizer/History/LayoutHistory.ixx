export module UIVisualizer.LayoutHistory;

import std;

// 레이아웃 JSON의 내용을 버전별로 메모리에 들고 있다가 되돌리기(Undo)와 다시 실행(Redo)에 쓴다 (구현: LayoutHistory.cpp)
// 버전 하나는 JSON을 성공적으로 읽은 때의 파일 내용 전체다. 누가 고쳤는지(사람의 이동이든 Claude의 수정이든)는 가리지 않는다.
// 되돌린 뒤 새 버전을 기록하면 그 뒤에 남아 있던 다시 실행 기록은 버린다. 앱을 끄면 사라진다.
export class LayoutHistory
{
public:
    static constexpr std::size_t kMaxSnapshots = 50;   // 넘으면 가장 오래된 버전부터 버린다

    // 지금 버전으로 text를 기록한다. 지금 버전과 같으면 아무것도 하지 않는다 (같은 내용을 다시 읽거나 되돌린 파일을 읽은 경우)
    void Record(std::string text);

    // 한 단계 앞(Undo) 또는 뒤(Redo)의 버전을 지금 버전으로 삼고 그 내용을 돌려준다. 더 갈 수 없으면 nullptr
    // 돌려준 내용은 이력 안의 것을 가리키므로 다음에 Record를 부르기 전까지만 쓸 수 있다 (복사하지 않으려는 것이다)
    const std::string* Undo();
    const std::string* Redo();

    bool CanUndo() const noexcept { return m_current > 0; }
    bool CanRedo() const noexcept { return m_current + 1 < m_snapshots.size(); }
    std::size_t Size() const noexcept { return m_snapshots.size(); }

private:
    std::vector<std::string> m_snapshots;   // 오래된 것부터
    std::size_t m_current{ 0 };             // 지금 버전의 위치 (m_snapshots가 비어 있지 않을 때만 의미가 있다)
};
