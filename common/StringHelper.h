// Convert a given std::string (containing UTF-8 chars) to a unicode string and vice versa.

#pragma once

#include <iostream>
#include <boost/noncopyable.hpp>

namespace uberstealth {

class StringToUnicode : public boost::noncopyable {
public:
	StringToUnicode(const std::string& str) :
		_s(str),
		_ws(NULL) {}
	
	~StringToUnicode() {
		delete[] _ws;
	}

	operator const wchar_t*();
	
private:
	const std::string& _s;
	wchar_t* _ws;
};

class UnicodeToString : public boost::noncopyable {
public:
	UnicodeToString(const std::wstring& str) :
		_s(str),
		_as(NULL) {};

	~UnicodeToString() {
		delete[] _as;
	}

	operator const char*();
	
private:
	const std::wstring& _s;
	char* _as;
};

}