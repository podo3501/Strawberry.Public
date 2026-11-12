export module DxRender.Task:Entry;

import std;
import :Node;
import :Context;
import :Handle;
import DxRender.Command;

export struct TaskEntry
{
    TaskNode task;
    TaskContext context;

    bool submitted{ false };
    bool started{ false };
    bool finished{ false };
    FenceID fenceID{ InvalidFenceID };

    std::atomic<int> activeDependents{ 0 }; // 나를 의존하는 자식 노드들 중, 아직 해제(Remove)되지 않고 살아있는 자식들의 총 개수. 이게 0이 되면 자신도 해제된다.

    TaskEntry() = default;
    TaskEntry(const TaskEntry&) = delete;
    TaskEntry& operator=(const TaskEntry&) = delete;

    // atomic 변수 때문에 이동 및 대입 연산자 작성
    TaskEntry(TaskEntry&& other) noexcept
        : task(std::move(other.task))
        , context(std::move(other.context))
        , submitted(other.submitted)
        , started(other.started)
        , finished(other.finished)
        , fenceID(other.fenceID)
    {
        activeDependents.store(other.activeDependents.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }

    TaskEntry& operator=(TaskEntry&& other) noexcept
    {
        if (this != &other)
        {
            task = std::move(other.task);
            context = std::move(other.context);
            submitted = other.submitted;
            started = other.started;
            finished = other.finished;
            fenceID = other.fenceID;

            activeDependents.store(other.activeDependents.load(std::memory_order_relaxed), std::memory_order_relaxed);
        }
        return *this;
    }
};
