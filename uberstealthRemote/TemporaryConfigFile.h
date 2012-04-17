// Creates a temporary file that is deleted on destruction of the object instance.

#pragma once

#include <iostream>
#include <boost/noncopyable.hpp>

class TemporaryConfigFile : public boost::noncopyable {
public:
	TemporaryConfigFile(const std::string& serializedConfig);
	~TemporaryConfigFile();
	std::string getFileName() const { return fileName_; }

private:
	std::string fileName_;
};
