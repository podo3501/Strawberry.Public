module;

#include <nlohmann/json.hpp>

export module Runtime.Serialization:Serializer;

import std;
import :Concepts;
import :ClassSerialize;

export class Serializer
{
	enum class Mode { Write, Read };

public:
	virtual ~Serializer() = default;
	Serializer() = delete;

	explicit Serializer(nlohmann::json& write) noexcept
		: m_mode{ Mode::Write }, m_write{ &write } {
	}

	explicit Serializer(const nlohmann::json& read) noexcept
		: m_mode{ Mode::Read }, m_read{ &read } {
	}

	bool IsWrite() const noexcept
	{
		return m_mode == Mode::Write;
	}

	// JsonOperation 분기 템플릿 처리 함수
	template<typename T>
	void Process(const std::string& key, T& data) noexcept
	{
		auto writeFunc = [this, &data](nlohmann::json& j) {
			if constexpr (HasSerialize<T>)
				SerializeClass(j, data); // SerializeIO 가 있는 클래스 (unique_ptr 포함)
			else if constexpr (SequenceLike<T>)
				SerializeSeqContainer(j, data); // 시퀀스 컨테이너
			else if constexpr (MapLike<T>)
				SerializeMapContainer(j, data); // Map 컨테이너
			else
				j = JsonTraits<T>::SerializeToJson(data); // 일반 데이터 형식
			};

		auto readFunc = [this, &data](const nlohmann::json& j) {
			if constexpr (HasSerialize<T>)
				DeserializeClass(j, data);
			else if constexpr (SequenceLike<T>)
				DeserializeSeqContainer(j, data);
			else if constexpr (MapLike<T>)
				DeserializeMapContainer(j, data);
			else
				data = JsonTraits<T>::DeserializeFromJson(j);
			};

		ProcessImpl(key, writeFunc, readFunc);
	}

private:
	template <typename ProcessFunc>
	static void ProcessWriteKey(const std::string& key, ProcessFunc processFunc, nlohmann::json& outData) noexcept
	{
		nlohmann::json writeJ{};
		processFunc(writeJ);
		outData[key] = std::move(writeJ);
	}

	template <typename ProcessFunc>
	static void ProcessReadKey(const std::string& key, ProcessFunc processFunc, const nlohmann::json& data) noexcept
	{
		if (!data.contains(key)) return;
		processFunc(data[key]);
	}

	template <typename WriteFunc, typename ReadFunc>
	void ProcessImpl(const std::string& key, WriteFunc&& writeFunc, ReadFunc&& readFunc) noexcept
	{
		if (IsWrite())
			ProcessWriteKey(key, writeFunc, *m_write);
		else
			ProcessReadKey(key, readFunc, *m_read);
	}

	Mode m_mode;
	nlohmann::json* m_write{ nullptr };       // 생성자에 전달된 json 포인터 보관
	const nlohmann::json* m_read{ nullptr };
};