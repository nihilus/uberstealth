// Represents a resource specified by an ID and a name in the resource section of a given module.

#pragma once

#include <Windows.h>
#include <iostream>

namespace uberstealth {

class ResourceItem {
public:
	ResourceItem(HMODULE hModule, int resourceID, const std::string& resourceType);
	void* getData() const;
	bool saveDataToFile(const std::string& fileName) const;
	size_t getDataSize() const { return size_; };

private:
	HMODULE hModule_;
	int resID_;
	std::string resType_;
	mutable size_t size_;
};

}