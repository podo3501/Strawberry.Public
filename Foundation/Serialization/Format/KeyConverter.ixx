export module Serialization:KeyConverter;

import std;
import Core.Utils;

// 파일에서 읽은 키를 바꾼다. 바꿀 수 없으면 nullopt이다 (예외를 던지지 않는다)
export template<typename K>
struct KeyConverter
{
	static std::string ToKey(const K& key) { return key; }
	static std::optional<K> FromKey(const std::string& key) { return key; }
};

template<>
struct KeyConverter<int>
{
	static std::string ToKey(const int& key) { return std::to_string(key); }
	static std::optional<int> FromKey(const std::string& key)
	{
		int value = 0;
		const auto result = std::from_chars(key.data(), key.data() + key.size(), value);
		if (result.ec != std::errc{} || result.ptr != key.data() + key.size())
			return std::nullopt;   // 숫자가 아니거나 뒤에 다른 글자가 남았다
		return value;
	}
};

template<>
struct KeyConverter<std::wstring>
{
	static std::string ToKey(const std::wstring& key) { return Core::WStringToString(key); }
	static std::optional<std::wstring> FromKey(const std::string& key) { return Core::StringToWString(key); }
};

export template<typename K>
std::string ToKeyString(const K& key) { return KeyConverter<K>::ToKey(key); }

export template<typename K>
std::optional<K> FromKeyString(const std::string& key) { return KeyConverter<K>::FromKey(key); }
