// Convert a given std::string (containing UTF-8 chars) to a unicode string and vice versa.

#pragma once

#include <boost/noncopyable.hpp>
#include <iostream>

namespace uberstealth {

class StringToUnicode : public boost::noncopyable
{
public:
	StringToUnicode(const std::string& str) :
		s_(str),
		ws_(NULL) {}
	
	~StringToUnicode()
	{
		delete[] ws_;
	}

	operator const wchar_t*();
	
private:
	const std::string& s_;
	wchar_t* ws_;
};

class UnicodeToString : public boost::noncopyable
{
public:
	UnicodeToString(const std::wstring& str) :
		s_(str),
		as_(NULL) {};

	~UnicodeToString()
	{
		delete[] as_;
	}

	operator const char*();
	
private:
	const std::wstring& s_;
	char* as_;
};

}