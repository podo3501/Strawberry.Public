module DxRender.Task;
	
import :Scheduler;
import :Types;
import :Node;
import :CommandLists;
import :Utils;
import Core.Assert;

TaskScheduler::TaskScheduler(CommandScheduler& cmdScheduler)
    : m_cmdScheduler{ cmdScheduler }
{}

TaskHandle TaskScheduler::AllocateHandle()
{
    return m_tasks.Emplace(TaskEntry{});
}

void TaskScheduler::SubmitTask(const std::vector<CompiledTask>& compiledTasks, std::shared_ptr<ResourceContext> resources)
{
    Core::Assert(!m_draining.load(std::memory_order_relaxed)); // Drain() 도중 SubmitTask 호출

    std::unordered_map<LocalTaskID, TaskHandle> remap; // RenderGraph에서 만든 일시적인 handle을 실제 사용가능한 task handle로 변경
    for (const auto& compiled : compiledTasks)
        remap[compiled.localId] = AllocateHandle();

    for (const auto& compiled : compiledTasks)
    {
        Core::Assert(compiled.task.type != CommandType::None);

        TaskHandle handle = remap[compiled.localId];
        TaskEntry* entry = m_tasks.Find(handle);
        Core::Assert(entry);
        Core::Assert(!entry->submitted);

        entry->task = compiled.task;
        entry->context.resources = resources;

        for (const auto& depLocalId : compiled.dependencies)
            entry->task.dependencies.push_back(remap[depLocalId]);
        for (const auto& depLocalId : compiled.dependents)
            entry->dependents.push_back(remap[depLocalId]);

        // 자신이 파괴될 때만 부모의 카운트를 깎아주면 되므로 std::erase가 필요 없어짐
        for (const auto& depHandle : entry->task.dependencies)
        {
            if (TaskEntry* parent = m_tasks.Find(depHandle))
                parent->activeDependents.fetch_add(1, std::memory_order_relaxed); // 부모들의 살아있는 자식 수 카운트를 1씩 증가
        }

        entry->submitted = true;
    }
}

void TaskScheduler::DeferRelease(std::vector<std::shared_ptr<void>> objects)
{
    Core::Assert(!m_draining.load(std::memory_order_relaxed));

    auto queue = m_cmdScheduler.GetQueue(CommandType::Direct);

    PendingRelease entry;
    entry.waitFenceID = queue->GetCurrentFence();
    entry.objects = std::move(objects);

    m_pendingReleases.push_back(std::move(entry));
}

void TaskScheduler::Execute()
{
    ProcessPendingReleases();

    std::vector<TaskHandle> toRemove;
    m_tasks.Visit([this, &toRemove](TaskHandle handle, TaskEntry& entry) {
        if (!entry.started)
        {
            if (!AreDependenciesDone(entry))
                return;

            if (!IsDirectFenceReady(entry)) // release task용
                return;

            ExecuteTask(entry);
        }

        if (!entry.finished && IsTaskFinished(entry))
            entry.finished = true;

        if (CanDeleteTask(entry))
            toRemove.push_back(handle);
        });

    for (const auto& handle : toRemove)
    {
        if (TaskEntry* entry = m_tasks.Find(handle))
            RemoveTask(handle, *entry);
    }
}

void TaskScheduler::Cancel(TaskHandle handle)
{
    TaskEntry* entry = m_tasks.Find(handle);
    if (!entry) return;

    // 자신이 취소되므로 내 부모들에게서 나의 자식 지분을 제거
    for (const auto& depHandle : entry->task.dependencies)
    {
        if (TaskEntry* parent = m_tasks.Find(depHandle))
            parent->activeDependents.fetch_sub(1, std::memory_order_relaxed);
    }

    for (const auto& childHandle : entry->dependents)
    {
        if (TaskEntry* child = m_tasks.Find(childHandle))
            std::erase(child->task.dependencies, handle); // 자식의 정방향 디펜던시에서 취소된 나를 지워줌
    }

    m_tasks.Remove(handle);
}

void TaskScheduler::Shutdown()
{
    m_draining.store(true, std::memory_order_relaxed); // 신규 제출 차단
    Drain(); // 모든 task가 정상적으로 실행되고 끝날 때까지 대기
    m_draining.store(false, std::memory_order_relaxed);
}

bool TaskScheduler::AreDependenciesDone(const TaskEntry& entry)
{
    for (const auto& dep : entry.task.dependencies)
    {
        const TaskEntry* depEntry = m_tasks.Find(dep);
        Core::Assert(depEntry); // task가 중간에 사라짐

        if (!depEntry->finished) return false;
    }
    return true;
}

bool TaskScheduler::IsDirectFenceReady(const TaskEntry& entry) const
{
    if (entry.waitFenceID == InvalidFenceID)
        return true;

    auto queue = m_cmdScheduler.GetQueue(CommandType::Direct);
    return queue->GetCompletedFence() >= entry.waitFenceID;
}

bool TaskScheduler::IsTaskFinished(const TaskEntry& entry)
{
    if (!entry.started) return false;
    return m_cmdScheduler.IsFenceComplete(entry.task.type, entry.fenceID);
}

void TaskScheduler::ExecuteTask(TaskEntry& entry)
{
    Core::Assert(entry.task.type != CommandType::None);
    Core::Assert(entry.task.numParallel <= 1); // Render용 pass 오실행 방어

    auto* queue = m_cmdScheduler.GetQueue(entry.task.type);
    CommandList* cmd = queue->Begin();
    if (!cmd) return;

    CommandList* cmds[] = { cmd };
    ExecuteTaskImmediate(cmds, entry.task, entry.context);
    entry.fenceID = queue->End();
    entry.started = true;
}

bool TaskScheduler::CanDeleteTask(const TaskEntry& entry)
{
    return entry.finished && (entry.activeDependents.load(std::memory_order_relaxed) == 0);
}

void TaskScheduler::RemoveTask(TaskHandle handle, TaskEntry& entry)
{
    for (const auto& depHandle : entry.task.dependencies)
    {
        if (TaskEntry* parent = m_tasks.Find(depHandle))
            parent->activeDependents.fetch_sub(1, std::memory_order_relaxed);
    }

    m_tasks.Remove(handle);
}

void TaskScheduler::ProcessPendingReleases()
{
    auto queue = m_cmdScheduler.GetQueue(CommandType::Direct);
    FenceID completed = queue->GetCompletedFence();

    std::erase_if(m_pendingReleases, [completed](const PendingRelease& object) {
        return completed >= object.waitFenceID;
        });
}

bool TaskScheduler::IsIdle() const noexcept
{
    return m_tasks.Empty() && m_pendingReleases.empty();
}

void TaskScheduler::Drain()
{
    while (!IsIdle())
    {
        Execute();
        std::this_thread::yield();
    }
}