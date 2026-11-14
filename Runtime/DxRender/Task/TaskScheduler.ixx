export module DxRender.Task:Scheduler;

import std;
import :Context;
import :Handle;
import :CompiledNode;
import :Entry;
import :CommandLists;
import Core.Handle;
import DxRender.Command;

struct PendingRelease
{
    FenceID waitFenceID{ InvalidFenceID };
    std::vector<std::shared_ptr<void>> objects;
};

export class TaskScheduler
{
public:
    ~TaskScheduler() = default;
    TaskScheduler() = delete;
    TaskScheduler(CommandScheduler& cmdScheduler);

    void SubmitTask(const std::vector<CompiledTask>& compiledTasks, std::shared_ptr<ResourceContext> resources);
    void DeferRelease(std::vector<std::shared_ptr<void>> objects);
    void Execute();
    void Shutdown();

private:
    TaskHandle AllocateHandle();
    bool AreDependenciesDone(const TaskEntry& entry);

    bool IsTaskFinished(const TaskEntry& entry);

    void ExecuteTask(TaskEntry& entry);
    void WaitOnOtherQueueDependencies(const TaskEntry& entry);
    bool CanDeleteTask(const TaskEntry& entry);
    void RemoveTask(TaskHandle handle, TaskEntry& entry);

    void ProcessPendingReleases();
    bool IsIdle() const noexcept;
    void Drain();

private:
    CommandScheduler& m_cmdScheduler;
    Core::HandlePool<TaskEntry, TaskTag> m_tasks;
    std::vector<PendingRelease> m_pendingReleases;

    std::atomic<bool> m_draining{ false };
};