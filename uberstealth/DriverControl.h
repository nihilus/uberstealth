// This class allows for starting/stopping kernel mode drivers.

#pragma once

#include <iostream>
#include "ResourceItem.h"

namespace uberstealth {

class DriverControl {
public:
	DriverControl();
	void startDriver(const ResourceItem& ri, const std::string& driverName);
	void stopDriver();
	void setMode(unsigned int ioctlCode, void* param, size_t paramSize) const;
	std::string getDriverPath() const { return driverPath_; }
	bool isRunning() const { return running_; }

private:
	void controlDriver(const std::string& driverPath, const std::string& driverName, bool load) const;
	void throwSysError(unsigned int lastError, const std::string& msg) const;
	bool running_;
	std::string driverPath_;
	std::string driverName_;
};

}