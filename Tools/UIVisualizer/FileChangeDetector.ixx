export module UIVisualizer.FileChangeDetector;

import std;

// 파일 하나의 수정 시각을 일정 간격으로 확인해서, 바뀐 뒤 잠깐 더 바뀌지 않으면 "바뀌었다"고 한 번 알려 준다.
// 편집기가 여러 번에 나눠 쓰는 도중의 반쪽짜리 파일을 읽지 않으려고 안정될 때까지 기다린다.
// 파일이 없으면 아무것도 알리지 않는다 (다시 만들어져서 수정 시각이 생기면 그때부터 본다).
export class FileChangeDetector
{
public:
    // path를 감시하기 시작한다. 지금 파일의 상태는 이미 처리한 것으로 본다 (시작할 때 읽은 내용)
    void Watch(std::filesystem::path path);

    // dt초가 지났음을 알린다. 파일이 바뀌고 안정되었으면 true (같은 변경에 한 번만)
    bool Poll(float dt);

    // 지금 파일의 상태를 처리한 것으로 기록한다. 이 도구가 직접 파일을 쓴 뒤에 부르면 그 변경은 다시 읽지 않는다
    void MarkHandled();

private:
    static constexpr float kCheckInterval = 0.1f;   // 초. 수정 시각을 확인하는 간격
    static constexpr float kSettleTime = 0.2f;      // 초. 바뀐 뒤 이 시간 동안 더 바뀌지 않으면 안정된 것으로 본다

    std::optional<std::filesystem::file_time_type> WriteTime() const;

    std::filesystem::path m_path;
    std::optional<std::filesystem::file_time_type> m_handled;    // 마지막으로 처리한 수정 시각
    std::optional<std::filesystem::file_time_type> m_observed;   // 마지막으로 본 수정 시각
    float m_stableFor{ 0.f };
    float m_sinceCheck{ 0.f };
};
