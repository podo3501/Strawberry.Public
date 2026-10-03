export module Runtime.Render.Provider:Budget;

import std;

// 사용자 정의 리터럴: MB 단위 변환
export constexpr std::size_t operator"" _MB(unsigned long long value) noexcept
{
    return static_cast<std::size_t>(value * 1024ull * 1024ull);
}

export struct BudgetRange
{
    float fastGpuMs;     // 최대 예산을 적용하는 GPU 시간
    float slowGpuMs;     // 최소 예산을 적용하는 GPU 시간
    std::size_t maxBudget; // fastGpuMs 이하일 때의 예산
    std::size_t minBudget; // slowGpuMs 이상일 때의 예산
};

// GPU 프레임 타임(gpuMs)에 따른 선형 보간 예산 계산
export inline std::size_t ComputeBudget(float gpuMs, const BudgetRange& range) noexcept
{
    if (gpuMs <= range.fastGpuMs)
        return range.maxBudget;

    if (gpuMs >= range.slowGpuMs)
        return range.minBudget;

    // fastGpuMs와 slowGpuMs 사이 구간 선형 보간 (Lerp)
    const float t = (gpuMs - range.fastGpuMs) / (range.slowGpuMs - range.fastGpuMs);
    const float interpolated = static_cast<float>(range.maxBudget) +
        t * static_cast<float>(static_cast<std::ptrdiff_t>(range.minBudget) - static_cast<std::ptrdiff_t>(range.maxBudget));

    return static_cast<std::size_t>(interpolated);
}

export namespace ProviderBudget
{
    constexpr BudgetRange Texture
    {
        .fastGpuMs = 5.0f,
        .slowGpuMs = 15.0f,
        .maxBudget = 32_MB,
        .minBudget = 4_MB
    };

    constexpr BudgetRange TextureCube
    {
        .fastGpuMs = 5.0f,
        .slowGpuMs = 15.0f,
        .maxBudget = 16_MB,
        .minBudget = 2_MB
    };

    constexpr BudgetRange Mesh
    {
        .fastGpuMs = 5.0f,
        .slowGpuMs = 15.0f,
        .maxBudget = 16_MB,
        .minBudget = 2_MB
    };
}