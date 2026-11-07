module UIVisualizer.FileChangeDetector;

import std;

void FileChangeDetector::Watch(std::filesystem::path path)
{
    m_path = std::move(path);
    MarkHandled();
}

bool FileChangeDetector::Poll(float dt)
{
    m_sinceCheck += dt;
    if (m_sinceCheck < kCheckInterval)
        return false;

    const float elapsed = m_sinceCheck;
    m_sinceCheck = 0.f;

    const auto time = WriteTime();
    if (!time)
        return false;

    if (time != m_observed)
    {
        m_observed = time;   // 바뀌는 중이다. 안정될 때까지 다시 센다
        m_stableFor = 0.f;
        return false;
    }

    if (time == m_handled)
        return false;

    m_stableFor += elapsed;
    if (m_stableFor < kSettleTime)
        return false;

    m_handled = time;
    return true;
}

void FileChangeDetector::MarkHandled()
{
    m_handled = WriteTime();
    m_observed = m_handled;
    m_stableFor = 0.f;
    m_sinceCheck = 0.f;
}

std::optional<std::filesystem::file_time_type> FileChangeDetector::WriteTime() const
{
    std::error_code error;
    const auto time = std::filesystem::last_write_time(m_path, error);
    if (error)
        return std::nullopt;
    return time;
}
