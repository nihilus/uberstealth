#include "StringHelper.h"
#include <Windows.h>

uberstealth::StringToUnicode::operator const wchar_t*() {
	if (!ws_) {
		size_t length = s_.length() + 1;
		int numberOfElements = MultiByteToWideChar(CP_UTF8, 0, s_.c_str(), static_cast<int>(length), 0, 0); 
		ws_ = new wchar_t[numberOfElements];
		MultiByteToWideChar(CP_UTF8, 0, s_.c_str(), static_cast<int>(length), ws_, numberOfElements);
	}
	return ws_;
}

uberstealth::UnicodeToString::operator const char*() {
	if (!as_) {
		size_t length = s_.length() + 1;
		int numberOfElements = WideCharToMultiByte(CP_UTF8, 0, s_.c_str(), static_cast<int>(length), NULL, 0, NULL, NULL);
		as_ = new char[numberOfElements];
		WideCharToMultiByte(CP_UTF8, 0, s_.c_str(), static_cast<int>(length), as_, numberOfElements, NULL, NULL);
	}
	return as_;
}