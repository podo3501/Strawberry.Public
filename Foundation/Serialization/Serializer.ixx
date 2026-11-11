module;

#include <nlohmann/json.hpp>

export module Serialization:Serializer;

import std;
import :Concepts;
import :ClassSerialize;
import Core.Logger;

export class Serializer
{
	enum class Mode { Write, Read };

public:
	virtual ~Serializer() = default;
	Serializer() = delete;

	explicit Serializer(nlohmann::json& write) noexcept
		: m_mode{ Mode::Write }, m_write{ &write } {
	}

	// parent는 이 객체를 포함하는 쪽의 Serializer다. 읽다가 생긴 오류가 parent에도 전해진다.
	// 읽을 json은 객체(키와 값)여야 하고 아니면 바로 오류다.
	explicit Serializer(const nlohmann::json& read, Serializer* parent = nullptr) noexcept
		: m_mode{ Mode::Read }, m_read{ &read }, m_parent{ parent }
	{
		if (!read.is_object())
			ReportError("객체가 아닌 JSON을 읽으려 함");
	}

	bool IsWrite() const noexcept
	{
		return m_mode == Mode::Write;
	}

	// 읽는 동안 형식이 틀린 값이나 객체가 아닌 JSON이 있었는가. 틀린 값은 건너뛰어 기존 값이 남는다
	bool HasError() const noexcept
	{
		return m_error;
	}

	// 오류를 로그로 남기고 이 객체와 부모들에 오류를 기록한다. 호출하는 쪽이 값의 의미를 검사해서 쓸 수도 있다
	void ReportError(std::string_view message) noexcept
	{
		Core::Logger::Error("Serializer: '{}' {}", m_key, message);
		for (Serializer* s = this; s; s = s->m_parent)
			s->m_error = true;
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
			{
				if (j.is_null())
					DeserializeClass(nlohmann::json::object(), data, this);   // 쓸 때 키가 하나도 없는 클래스는 null로 적힌다
				else if (!j.is_object())
					ReportError("객체가 아닌 값");
				else
					DeserializeClass(j, data, this);
			}
			else if constexpr (SequenceLike<T>)
				DeserializeSeqContainer(j, data, this);
			else if constexpr (MapLike<T>)
				DeserializeMapContainer(j, data, this);
			else
			{
				auto value = JsonTraits<T>::DeserializeFromJson(j);
				if (value)
					data = std::move(*value);
				else
					ReportError("값의 형식이 틀림");
			}
			};

		m_key = key;
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
	Serializer* m_parent{ nullptr };
	std::string m_key;                         // 지금 처리 중인 키 (오류 로그에 쓴다)
	bool m_error{ false };
};
