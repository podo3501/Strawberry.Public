export module Runtime.Serialization:KeyConverter;

import std;
import Core.Utils;

export template<typename K>
struct KeyConverter
{
	static std::string ToKey(const K& key) { return key; }
	static K FromKey(const std::string& key) { return key; }
};

export template<>
struct KeyConverter<int>
{
	static std::string ToKey(const int& key) { return std::to_string(key); }
	static int FromKey(const std::string& key) { return std::stoi(key); }
};

export template<>
struct KeyConverter<std::wstring>
{
	static std::string ToKey(const std::wstring& key) { return Core::WStringToString(key); }
	static std::wstring FromKey(const std::string& key) { return Core::StringToWString(key); }
};

export template<typename K>
std::string ToKeyString(const K& key) { return KeyConverter<K>::ToKey(key); }

export template<typename K>
K FromKeyString(const std::string& key) { return KeyConverter<K>::FromKey(key); }