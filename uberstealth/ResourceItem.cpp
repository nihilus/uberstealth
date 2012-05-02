#include "ResourceItem.h"
#include <common/StringHelper.h>

uberstealth::ResourceItem::ResourceItem(HMODULE hModule, int resourceID, const std::string& resourceType) :
	hModule_(hModule),
	resID_(resourceID),
	resType_(resourceType) {}

void* uberstealth::ResourceItem::getData() const {
	HRSRC hResInfo = FindResource(hModule_, (LPCWSTR)resID_, uberstealth::StringToUnicode(resType_));
	if (!hResInfo) {
		return NULL;
	}

	HGLOBAL resData = LoadResource(hModule_, hResInfo);
	if (!resData) {
		return NULL;
	}

	void* dataPtr = LockResource(resData);
	size_ = 0;
	if (dataPtr) size_ = SizeofResource(hModule_, hResInfo);
	return dataPtr;
}

bool uberstealth::ResourceItem::saveDataToFile(const std::string& fileName) const {
	void* dataPtr = getData();
	if (!dataPtr) {
		return false;
	}

	FILE* hFile;
	fopen_s(&hFile, fileName.c_str(), "wb");
	if (!hFile) {
		return false;
	}

	bool retVal = false;
	if (fwrite(dataPtr, size_, 1, hFile)) {
		retVal = true;
	}
	fclose(hFile);

	return retVal;	
}