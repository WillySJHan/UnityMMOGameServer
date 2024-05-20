#pragma once
#include <codecvt>
#include <locale>
#include <random>

class Utility
{
public:
	static std::string WstrTostr(std::wstring wstr)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		std::string str = converter.to_bytes(wstr);

		return str;
	}

	static std::wstring StrTowstr(std::string str)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		std::wstring wstr = converter.from_bytes(str);

		return wstr;
	}

	template<typename T>
	static T GetRandom(T min, T max)
	{
		std::random_device randomDevice;
		std::mt19937 generator(randomDevice());

		if constexpr (std::is_integral_v<T>)
		{
			std::uniform_int_distribution<T> distribution(min, max);
			return distribution(generator);
		}
		else
		{
			std::uniform_real_distribution<T> distribution(min, max);
			return distribution(generator);
		}
	}

};
