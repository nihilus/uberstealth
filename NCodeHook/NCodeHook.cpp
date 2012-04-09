#include "NCodeHook.h"
#include <Windows.h>

const unsigned int MaxInstructions = 20;

static const unsigned int TrampolineBufferSize = 4096;

template <typename ArchT>
NCodeHook<ArchT>::NCodeHook(bool cleanOnDestruct) :
	MaxTotalTrampolineSize(ArchT::AbsJumpPatchSize + ArchT::MaxTrampolineSize),
	_cleanOnDestruct(cleanOnDestruct),
	_forceAbsJmp(false) {
	_trampolineBuffer = VirtualAlloc(NULL, TrampolineBufferSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (_trampolineBuffer == NULL) throw std::runtime_error("Unable to allocate trampoline memory!");
	for (uintptr_t i=(uintptr_t)_trampolineBuffer; i<(uintptr_t)_trampolineBuffer+TrampolineBufferSize; i+=MaxTotalTrampolineSize)
		_freeTrampolines.insert(i);
}

template <typename ArchT>
NCodeHook<ArchT>::~NCodeHook() {
	if (_cleanOnDestruct) {
		// Restore all hooks and free memory.
		for (size_t i = _hookedFunctions.size(); i > 0; --i) removeHook(_hookedFunctions[i - 1]);
		VirtualFree(_trampolineBuffer, 0, MEM_RELEASE);
	}
}

template <typename ArchT>
bool NCodeHook<ArchT>::isBranch(const char* instr) {
	if (instr[0] == 'J' || strstr(instr, "CALL"))
		return true;
	else return false;
}

template <typename ArchT>
int NCodeHook<ArchT>::getMinOffset(const unsigned char* codePtr, unsigned int jumpPatchSize) {
	_DecodeResult result;
	_DecodedInst instructions[MaxInstructions];
	unsigned int instructionCount = 0;

	result = distorm_decode(0, codePtr, 20, ArchT::DisasmType, instructions, MaxInstructions, &instructionCount);
	if (result != DECRES_SUCCESS) return -1;

	unsigned int offset = 0;
	for (unsigned int i = 0; offset < jumpPatchSize && i < instructionCount; ++i) {
		if (isBranch((const char*)instructions[i].mnemonic.p)) return -1;
		offset += instructions[i].size;
	}
	// If we were unable to disassemble enough instructions we fail.
	if (offset < jumpPatchSize) return -1;

	return offset;
}

// Create a new hook for "hookFunc" and return the trampoline which can be used to
// call the original function without the hook.
template <typename ArchT>
template <typename U> 
U NCodeHook<ArchT>::createHook(U originalFunc, U hookFunc) {
	// check if this function is already hooked
	std::map<uintptr_t, NCodeHookItem>::const_iterator cit = _hookedFunctions.begin();
	while(cit != _hookedFunctions.end()) {
		if ((uintptr_t)cit->second.OriginalFunc == (uintptr_t)originalFunc) return (U)cit->second.Trampoline;
		++cit;
	}

	bool useAbsJump = _forceAbsJmp;
	int offset = 0;
	if (useAbsJump || _architecture.requiresAbsJump((uintptr_t)originalFunc, (uintptr_t)hookFunc)) {
		offset = getMinOffset((const unsigned char*)originalFunc, ArchT::AbsJumpPatchSize);
		useAbsJump = true;
	}		
	else offset = getMinOffset((const unsigned char*)originalFunc, ArchT::NearJumpPatchSize);
	if (offset == -1) return false;

	DWORD oldProtect = 0;
	BOOL retVal = VirtualProtect((LPVOID)originalFunc, ArchT::MaxTrampolineSize, PAGE_EXECUTE_READWRITE, &oldProtect);
	if (!retVal) return false;

	// Get trampoline memory and copy instructions to trampoline.
	uintptr_t trampolineAddr = getFreeTrampoline();
	memcpy((void*)trampolineAddr, (void*)originalFunc, offset);
	if (useAbsJump)	{
		_architecture.writeAbsJump((uintptr_t)originalFunc, (uintptr_t)hookFunc);
		_architecture.writeAbsJump(trampolineAddr + offset, (uintptr_t)originalFunc + offset);
	}
	else {
		_architecture.writeNearJump((uintptr_t)originalFunc, (uintptr_t)hookFunc);
		_architecture.writeNearJump(trampolineAddr + offset, (uintptr_t)originalFunc + offset);
	}

	DWORD dummy;
	VirtualProtect((LPVOID)originalFunc, ArchT::MaxTrampolineSize, oldProtect, &dummy);

	FlushInstructionCache(GetCurrentProcess(), (LPCVOID)trampolineAddr, MaxTotalTrampolineSize);
	FlushInstructionCache(GetCurrentProcess(), (LPCVOID)originalFunc, useAbsJump ? ArchT::AbsJumpPatchSize : ArchT::NearJumpPatchSize);
	
	NCodeHookItem item((uintptr_t)originalFunc, (uintptr_t)hookFunc, trampolineAddr, offset);
	_hookedFunctions.insert(std::make_pair((uintptr_t)hookFunc, item));

	return (U)trampolineAddr;
}

template <typename ArchT>
template <typename U> 
U NCodeHook<ArchT>::createHookByName(const std::string& dll, const std::string& funcName, U newFunc) {
	U funcPtr = NULL;
	HMODULE hDll = LoadLibraryA(dll.c_str());
	funcPtr = (U)GetProcAddress(hDll, funcName.c_str());
	if (funcPtr != NULL) funcPtr = createHook(funcPtr, newFunc);
	FreeLibrary(hDll);
	return funcPtr;
}

template <typename ArchT>
template <typename U>
bool NCodeHook<ArchT>::removeHook(U address) {
	// Remove hooked function again, address points to the HOOK function!
	std::map<uintptr_t, NCodeHookItem>::const_iterator result = _hookedFunctions.find((uintptr_t)address);
	if (result != _hookedFunctions.end())
		return removeHook(result->second);
	return true;
}

template <typename ArchT>
bool NCodeHook<ArchT>::removeHook(NCodeHookItem item) {
	// Copy overwritten instructions back to original function.
	DWORD oldProtect;
	BOOL retVal = VirtualProtect((LPVOID)item.OriginalFunc, item.PatchSize, PAGE_EXECUTE_READWRITE, &oldProtect);
	if (!retVal) return false;
	memcpy((void*)item.OriginalFunc, (const void*)item.Trampoline, item.PatchSize);
	DWORD dummy;
	VirtualProtect((LPVOID)item.OriginalFunc, item.PatchSize, oldProtect, &dummy);
	
	_hookedFunctions.erase(item.HookFunc);
	_freeTrampolines.insert(item.Trampoline);
	FlushInstructionCache(GetCurrentProcess(), (LPCVOID)item.OriginalFunc, item.PatchSize);
	
	return true;
}

template <typename ArchT>
uintptr_t NCodeHook<ArchT>::getFreeTrampoline() {
	if (_freeTrampolines.empty()) throw std::runtime_error("No trampoline space available!");
	std::set<uintptr_t>::iterator it = _freeTrampolines.begin();
	uintptr_t result = *it;
	_freeTrampolines.erase(it);
	return result;
}