// The IDA specific functions required to implement uberstealth.

#pragma once

#include <Windows.h>
#include <boost/unordered_set.hpp>
#include "IDACommon.h"
#include <iostream>

// Template specialization needed for boost::unordered_set.
namespace boost {
	template<>
	struct hash<exception_info_t> {
		size_t operator()(const exception_info_t& exceptionInfo) const {
			size_t hash = 0;
			boost::hash_combine(hash, exceptionInfo.code);
			boost::hash_combine(hash, exceptionInfo.flags);
			boost::hash_range(hash, exceptionInfo.name.begin(), exceptionInfo.name.end());
			boost::hash_range(hash, exceptionInfo.desc.begin(), exceptionInfo.desc.end());
			return hash;
		}
	};
}

// Template specialization needed for boost::unordered_set.
namespace std {
	template<>
	struct equal_to<exception_info_t> : public std::unary_function<exception_info_t, bool> {
		bool operator()(const exception_info_t& lhs, const exception_info_t& rhs) const {
			bool found = lhs.code == rhs.code &&
				lhs.flags == rhs.flags &&
				lhs.name == rhs.name &&
				lhs.desc == rhs.desc;
			return found;
		}
	};
}

namespace uberstealth {
	
class IDALogger {
public:
	IDALogger();
	void logString(const char* str, ...) const;

private:
	HWND hIDAWnd_;
	unsigned int idaMainThread_;
};

class IDAEngine {
public:
	~IDAEngine();
	bool setBreakpoint(uintptr_t address) const;
	bool removeBreakpoint(uintptr_t address) const;
	void setExceptionOption(unsigned int exceptionCode, bool ignore);
	bool continueProcess() const;

private:	
	struct ExceptionFilter 	{
		ExceptionFilter(const boost::unordered_set<exception_info_t>* addedExceptions) :
			addedExceptions_(addedExceptions) {}

		bool operator()(const exception_info_t& exceptionInfo) const {
			return addedExceptions_->find(exceptionInfo) != addedExceptions_->end();
		}
		const boost::unordered_set<exception_info_t>* addedExceptions_;
	};

	void showExceptionDialog(bool ignoreException) const;
	const exception_info_t* uberstealth::IDAEngine::findException(unsigned int exceptionCode) const;
	void restoreExceptions();
	
	boost::unordered_set<exception_info_t> addedExceptions_;
};

}