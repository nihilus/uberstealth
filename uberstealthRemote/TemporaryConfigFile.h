// Creates a temporary file that is deleted on destruction of the object instance.

#pragma once

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>

class TemporaryConfigFile : public boost::noncopyable {
public:
	TemporaryConfigFile(const std::string& serializedConfig);
	~TemporaryConfigFile();
	boost::filesystem::path getFileName() const { return fileName_; }

private:
	boost::filesystem::path fileName_;
};