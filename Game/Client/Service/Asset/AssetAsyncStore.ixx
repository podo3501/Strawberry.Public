export module Service.Asset:AsyncStore;

import std;
import Service.AssetAsyncTypes;
import Core.Assert;

export template<typename T>
class AssetAsyncStore
{
public:
	AssetRequestID Push(T value)
	{
		const AssetRequestID id = ++m_nextID;
		m_entries.emplace(id, std::move(value));

		return id;
	}

	template<typename... Args>
	AssetRequestID Emplace(Args&&... args)
	{
		const AssetRequestID id = ++m_nextID;
		m_entries.emplace(id, T{ std::forward<Args>(args)... });

		return id;
	}

	void Insert(AssetRequestID id, T value)
	{
		auto [_, inserted] = m_entries.emplace(id, std::move(value));
		Core::Assert(inserted);
	}

	std::optional<T> Take(AssetRequestID id)
	{
		auto it = m_entries.find(id);
		if (it == m_entries.end())
			return std::nullopt;

		T value = std::move(it->second);
		m_entries.erase(it);

		return value;
	}

	bool Contains(AssetRequestID id) const
	{
		return m_entries.find(id) != m_entries.end();
	}

	std::size_t Size() const { return m_entries.size(); }
	void Clear() { m_entries.clear(); }

private:
	std::unordered_map<AssetRequestID, T> m_entries;
	std::atomic<AssetRequestID> m_nextID{ InvalidAssetRequestID };
};
