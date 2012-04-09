#include "StringHelper.h"
#include <Windows.h>

uberstealth::StringToUnicode::operator const wchar_t*() {
	if (!_ws) {
		size_t length = _s.length() + 1;
		int numberOfElements = MultiByteToWideChar(CP_UTF8, 0, _s.c_str(), static_cast<int>(length), 0, 0); 
		_ws = new wchar_t[numberOfElements];
		MultiByteToWideChar(CP_UTF8, 0, _s.c_str(), static_cast<int>(length), _ws, numberOfElements);
	}
	return _ws;
}

uberstealth::UnicodeToString::operator const char*() {
	if (!_as) {
		size_t length = _s.length() + 1;
		int numberOfElements = WideCharToMultiByte(CP_UTF8, 0, _s.c_str(), static_cast<int>(length), NULL, 0, NULL, NULL);
		_as = new char[numberOfElements];
		WideCharToMultiByte(CP_UTF8, 0, _s.c_str(), static_cast<int>(length), _as, numberOfElements, NULL, NULL);
	}
	return _as;
}