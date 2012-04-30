// Defines the data structure that is read by the injected dll via IPC.
// This data structure contains the original PE header of the debuggee as well as the image base address.

#pragma once

#include <Windows.h>
#include <iostream>

namespace uberstealth {
	static const char* ConfigFileDataStr = "HideDebuggerPathStrData212";
	static const char* IDAProcessIDStr = "IDAProcessID";
	static const char* PEHeaderDataStr = "PEHeaderData";
	static const char* PERestoreRequiredStr = "PERestore";
	static const size_t ConfigDataSegmentSize = MAX_PATH;
	static const size_t SegmentSize = 4096;

	std::string getSegmentName(unsigned int processID);
	std::string getSegmentName();
	
	// TODO(jan.newger@newgre.net): store everything else in this struct as well (path, profile, etc).
	struct IPCPEHeaderData {
		IPCPEHeaderData() : imageBase(0) {}
		IPCPEHeaderData(uintptr_t base, const IMAGE_NT_HEADERS& headers) :
			imageBase(base),
			ntHeaders(headers) {}

		uintptr_t imageBase;
		IMAGE_NT_HEADERS ntHeaders;
	};
}