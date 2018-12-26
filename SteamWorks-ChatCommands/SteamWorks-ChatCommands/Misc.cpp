
#include "Misc.h"

std::string ws2s(const std::wstring &str)
{
	size_t sz = str.length();

	int nd = WideCharToMultiByte(CP_UTF8, 0, &str[0], sz, NULL, 0, NULL, NULL);
	std::string ret(nd, 0);
	int w = WideCharToMultiByte(CP_UTF8, 0, &str[0], sz, &ret[0], nd, NULL, NULL);
	if (w != sz)
	{
		return "Invalid size written";
	}

	return ret;
}

std::wstring s2ws(const std::string &str)
{
	size_t sz = str.length();
	int nd = MultiByteToWideChar(CP_UTF8, 0, &str[0], sz, NULL, 0);
	std::wstring ret(nd, 0);
	int w = MultiByteToWideChar(CP_UTF8, 0, &str[0], sz, &ret[0], nd);

	if (w != sz)
		return L"Invalid size written";


	return ret;
}
