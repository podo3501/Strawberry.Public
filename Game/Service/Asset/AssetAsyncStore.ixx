export module Service.Asset:AsyncStore;

import std;
import Contract.Common.AssetAsyncTypes;
import Core.Assert;

// ID를 매겨 값을 보관한다. 스스로 잠그지 않으므로 쓰는 쪽이 mutex로 감싼다.
export template<typename T>
class AssetAsyncStore
{
public:
	template<typename... Args>
	AssetRequestID Emplace(Args&&... args)
	{
		const AssetRequestID id = ++m_nextID;
		auto [_, inserted] = m_entries.emplace(id, T{ std::forward<Args>(args)... });
		Core::Assert(inserted);

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

private:
	std::unordered_map<AssetRequestID, T> m_entries;
	AssetRequestID m_nextID{ InvalidAssetRequestID };
};
