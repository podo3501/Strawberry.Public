export module Runtime.Render.Provider:PendingLoadQueue;

import std;
import Runtime.Render.Resource;

export class PendingLoadQueue
{
public:
    void Add(std::shared_ptr<IPendingResource> res)
    {
        m_pendingLoads.push_back(std::move(res));
    }

    void Flush()
    {
        for (auto it = m_pendingLoads.begin(); it != m_pendingLoads.end();)
        {
            auto& loadRes = *it;
            if (!loadRes->IsDependencyReady())
            {
                ++it;
                continue;
            }
            loadRes->MarkReady();
            it = m_pendingLoads.erase(it);
        }
    }

private:
    std::vector<std::shared_ptr<IPendingResource>> m_pendingLoads;
};