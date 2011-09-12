#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <ntstrsafe.h>

bool initDeviceNames(PUNICODE_STRING registryPath);

#ifdef __cplusplus
}; // extern "C"
#endif

extern UNICODE_STRING DynamicDeviceName;
extern UNICODE_STRING DynamicDosDeviceName;