#include <boost/filesystem.hpp>
#include "DriverControl.h"
#include <sstream>
#include <common/StringHelper.h>
#include <Windows.h>

// The driver is NOT unloaded if the class is destroyed because the user might
// want to use it although this controller class has gone out of scope.
DriverControl::DriverControl() :
	running_(false) {}

// start or stop given driver
void DriverControl::controlDriver(const std::string& driverPath, const std::string& driverName, bool load) const {
	using uberstealth::StringToUnicode;
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCManager == NULL) {
		std::string msg;
		msg = "Error while trying to control driver (" + driverName + ")";
		msg += ": unable to open service manager";
		throwSysError(GetLastError(), msg);
	}

	SC_HANDLE hService = CreateService(hSCManager, StringToUnicode(driverName), StringToUnicode(driverName),
		SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, StringToUnicode(driverPath), NULL, NULL, NULL, NULL, NULL);
	DWORD lastError = 0;
	if (!hService) {
		lastError = GetLastError();
		if (lastError == ERROR_SERVICE_EXISTS) {
			hService = OpenService(hSCManager, StringToUnicode(driverName), SERVICE_ALL_ACCESS);
		} else {
			CloseServiceHandle(hSCManager);
			throwSysError(lastError, "Error while trying to create driver service (" + driverName + ")");
		}
	}

	if (load) {
		if (!StartService(hService, 0, NULL)) {
			lastError = GetLastError();
			if (lastError != ERROR_SERVICE_ALREADY_RUNNING) {
				CloseServiceHandle(hSCManager);
				CloseServiceHandle(hService);
				throwSysError(lastError, "Error while trying to start driver service (" + driverName + ")");
			}
		}
	} else {
		SERVICE_STATUS ss;
		ControlService(hService, SERVICE_CONTROL_STOP, &ss);
		if (!DeleteService(hService)) {
			lastError = GetLastError();
			CloseServiceHandle(hSCManager);
			CloseServiceHandle(hService);
			throwSysError(lastError, "Error while trying to stop driver (" + driverName + ")");
		}
	}

	CloseServiceHandle(hSCManager);
	CloseServiceHandle(hService);
}

// extracts the driver file and starts it with the given name
// if no name is given, a random name is generated
void DriverControl::startDriver(const ResourceItem& ri, const std::string& driverName) {
	if (running_) {
		return;
	}

	if (driverName.empty()) {
		throw std::runtime_error("Error while trying to start driver: cannot start driver with empty name.");
	}
	
	wchar_t tmpPath[MAX_PATH];
	GetTempPath(MAX_PATH, tmpPath);
	boost::filesystem::path p(tmpPath);
	p /= driverName + ".sys";
	if (ri.saveDataToFile(p.string())) {
		controlDriver(p.string(), driverName, true);
		driverName_ = driverName;
		driverPath_ = p.string();
		running_ = true;
	}
	else {
		throw std::runtime_error("Error while trying to save driver to file: " + p.string());
	}
}

void DriverControl::stopDriver() {
	if (!running_) {
		return;
	}
	controlDriver(driverPath_, driverName_, false);
	boost::filesystem::remove(driverPath_);
	running_ = false;
}

// Send IOCTL command to driver.
void DriverControl::setMode(unsigned int ioctlCode, void* param, size_t paramSize) const {
	std::string device = "\\\\.\\" + driverName_;
	HANDLE hDevice = CreateFile(uberstealth::StringToUnicode(device), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDevice != INVALID_HANDLE_VALUE) {
		DWORD bytesReturned;
		if (!DeviceIoControl(hDevice, ioctlCode, param, paramSize, NULL, 0, &bytesReturned, NULL)) {
			DWORD lastErr = GetLastError();
			CloseHandle(hDevice);
			throwSysError(lastErr, "Unable to send IOCTL command to driver (" + driverName_ + ")");
		}
		CloseHandle(hDevice);
	} else {
		DWORD lastErr = GetLastError();
		throwSysError(lastErr, "Unable to open driver object (" + driverName_ + ")");
	}
}

void DriverControl::throwSysError(unsigned int lastError, const std::string& msg) const {
	std::ostringstream oss;
	oss << msg << ", system error code was: " << lastError;
	throw std::runtime_error(oss.str());
}