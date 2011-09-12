#include "DynamicDriverName.h"

UNICODE_STRING DynamicDeviceName = {0, 0, 0};
UNICODE_STRING DynamicDosDeviceName = {0, 0, 0};

PCWSTR devPrefix = L"\\Device\\";
PCWSTR dosPrefix = L"\\DosDevices\\";

bool buildDeviceName(PUNICODE_STRING dest, PCWSTR namePrefix, PCWSTR devName)
{
	size_t bufLen = (wcslen(namePrefix) + wcslen(devName) + 1) * sizeof(WCHAR);
	PWSTR devNameBuf = (PWSTR)ExAllocatePool(PagedPool, bufLen);
	if (!devNameBuf) return false;
	if (dest->Buffer) ExFreePool(dest->Buffer);

	RtlStringCbCopyW(devNameBuf, bufLen, namePrefix);
	RtlStringCbCatW(devNameBuf, bufLen, devName);
	RtlInitUnicodeString(dest, devNameBuf);
	return true;
}

// we get our own name so we can create the device objects based on this
// this allows us to be started with a custom name
PWSTR getNameFromRegistry(PUNICODE_STRING registryPath)
{
	UNICODE_STRING registryKey;
	UNICODE_STRING valueName;
	OBJECT_ATTRIBUTES objAttr;
	HANDLE hKey;

	RtlInitUnicodeString(&valueName, L"DisplayName");
	InitializeObjectAttributes(&objAttr, registryPath, OBJ_KERNEL_HANDLE, NULL, NULL);

	PWSTR result = NULL;

	NTSTATUS status = ZwOpenKey(&hKey, KEY_READ, &objAttr);
	if (NT_SUCCESS(status))
	{
		ULONG size = 0;
		status = ZwQueryValueKey(hKey, &valueName, KeyValuePartialInformation, NULL, 0, &size);
		if (status == STATUS_BUFFER_TOO_SMALL && size > 0)
		{ 
			PKEY_VALUE_PARTIAL_INFORMATION vpip = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePool(PagedPool, size);
			if (vpip)
			{
				status = ZwQueryValueKey(hKey, &valueName, KeyValuePartialInformation, vpip, size, &size);
				if (NT_SUCCESS(status))
				{
					result = (PWSTR)ExAllocatePool(PagedPool, vpip->DataLength);
					RtlStringCbCopyW(result, vpip->DataLength, (PCWSTR)vpip->Data);
					ExFreePool(vpip);
				}
			}
		}
		ZwClose(hKey);
	}

	return result;
}

// build device driver names by examining the drivers' registry path
bool initDeviceNames(PUNICODE_STRING registryPath)
{
	PWSTR customName = getNameFromRegistry(registryPath);
	if (!customName) return false;
	bool retVal = false;
	
	if (buildDeviceName(&DynamicDeviceName, devPrefix, customName)
		&& buildDeviceName(&DynamicDosDeviceName, dosPrefix, customName))
	{
		retVal = true;
	}
	ExFreePool(customName);
	return retVal;
}