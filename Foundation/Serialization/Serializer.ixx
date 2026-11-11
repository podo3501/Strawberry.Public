module;

#include <nlohmann/json.hpp>

export module Serialization:Serializer;

import std;
import Core.Logger;

// 읽기와 쓰기를 같은 Serialize 코드로 처리한다. ForRead/ForWrite로 모드를 정해 만든다 (Process의 본문은 ValueIO에 있다).
export class Serializer
{
	enum class Mode { Write, Read };

public:
	// 목록과 표의 항목 하나를 처리하는 동안 그 항목의 이름(번호나 표의 키)을 오류 경로에 넣는다
	class ItemScope
	{
	public:
		ItemScope(Serializer& serializer, std::string item) : m_serializer{ serializer }
		{
			m_serializer.m_items.push_back(std::move(item));
		}

		~ItemScope()
		{
			m_serializer.m_items.pop_back();
		}

		ItemScope(const ItemScope&) = delete;
		ItemScope& operator=(const ItemScope&) = delete;

	private:
		Serializer& m_serializer;
	};

	Serializer() = delete;
	Serializer(const Serializer&) = delete;
	Serializer& operator=(const Serializer&) = delete;

	// parent는 이 객체를 포함하는 쪽의 Serializer다. 쓰다가 생긴 오류가 parent에도 전해진다.
	// 쓸 json은 객체이거나 null(쓰면 객체가 됨)이어야 하고 아니면 바로 오류이며 아무것도 쓰지 않는다.
	static Serializer ForWrite(nlohmann::json& target, Serializer* parent = nullptr) noexcept
	{
		return Serializer{ &target, nullptr, parent };
	}

	// 읽을 json은 객체(키와 값)여야 하고 아니면 바로 오류다. 읽는 동안 살아 있어야 하므로 임시 json은 받지 않는다.
	static Serializer ForRead(const nlohmann::json& source, Serializer* parent = nullptr) noexcept
	{
		return Serializer{ nullptr, &source, parent };
	}
	static Serializer ForRead(nlohmann::json&&, Serializer* = nullptr) = delete;

	bool IsWrite() const noexcept
	{
		return m_mode == Mode::Write;
	}

	// 읽기나 쓰기 중 오류가 있었는가. 읽을 때 틀린 값은 건너뛰어 기존 값이 남는다
	bool HasError() const noexcept
	{
		return m_error;
	}

	// 루트부터 지금 처리 중인 키까지의 경로다 ("inner/value", 목록과 표 안은 "list/1/value"와 "table/a/value"). 오류 로그에 쓴다
	std::string Path() const
	{
		std::vector<const Serializer*> chain;
		for (const Serializer* s = this; s; s = s->m_parent)
			chain.push_back(s);

		std::string path;
		const auto append = [&path](const std::string& segment) {
			if (segment.empty())
				return;
			if (!path.empty())
				path += '/';
			path += segment;
			};
		for (auto it = chain.rbegin(); it != chain.rend(); ++it)
		{
			append((*it)->m_key);
			for (const auto& item : (*it)->m_items)
				append(item);
		}
		return path;
	}

	// 오류를 로그로 남기고 이 객체와 부모들에 오류를 기록한다. 호출하는 쪽이 값의 의미를 검사해서 쓸 수도 있다
	void ReportError(std::string_view message) noexcept
	{
		Core::Logger::Error("Serializer: '{}' {}", Path(), message);
		for (Serializer* s = this; s; s = s->m_parent)
			s->m_error = true;
	}

	// 쓰기면 key에 값을 쓰고, 읽기면 key가 있을 때만 읽는다 (없으면 기존 값이 남는다)
	template<typename T>
	void Process(const std::string& key, T& data) noexcept;

private:
	Serializer(nlohmann::json* write, const nlohmann::json* read, Serializer* parent) noexcept
		: m_mode{ write ? Mode::Write : Mode::Read }, m_write{ write }, m_read{ read }, m_parent{ parent }
	{
		if (write && !write->is_object() && !write->is_null())
			ReportError("객체가 아닌 JSON에 쓰려 함");
		else if (read && !read->is_object())
			ReportError("객체가 아닌 JSON을 읽으려 함");
	}

	Mode m_mode;
	nlohmann::json* m_write{ nullptr };       // 만들 때 전달된 json 포인터 보관
	const nlohmann::json* m_read{ nullptr };
	Serializer* m_parent{ nullptr };
	std::string m_key;                         // 지금 처리 중인 키 (오류 로그의 경로에 쓴다)
	std::vector<std::string> m_items;          // 목록과 표를 처리하는 동안 지금 항목의 번호나 키
	bool m_error{ false };
};
