#pragma warning (disable : 6387)
#include "Memory.h"

template<typename ... Arg>
uint64_t callHook(const Arg ... args)
{
	void* hookedFunc = GetProcAddress(LoadLibrary(L"win32u.dll"), "NtDxgkVailDisconnect");
	auto func = static_cast<uint64_t(_stdcall*)(Arg...)>(hookedFunc);
	return func(args ...);
}


ULONG64 GetModuleBaseAddress(const char* moduleName)
{
	MEMSTRUCT mem = { 0 };
	mem.pid = processId;
	mem.moduleName = moduleName;

	mem.getBaseAdress = 1;
	mem.getModSize = 0;
	mem.read = 0;
	mem.write = 0;
	
	callHook(&mem);

	ULONG64 base = 0;
	base = mem.baseAddress;

	return base;
}

ULONG64 GetModuleSize(const char* moduleName)
{
	MEMSTRUCT mem = { 0 };
	mem.pid = processId;
	mem.moduleName = moduleName;

	mem.getBaseAdress = 0;
	mem.getModSize = 1;
	mem.read = 0;
	mem.write = 0;

	callHook(&mem);

	ULONG64 size = 0;
	size = mem.modSize;

	return size;
}

bool WriteMem(UINT_PTR writeAddress, UINT_PTR sourceAddress, SIZE_T writeSize)
{
	MEMSTRUCT mem = { 0 };
	mem.pid = processId;
	mem.address = writeAddress;

	mem.getBaseAdress = 0;
	mem.getModSize = 0;
	mem.write = 1;
	mem.read = 0;
	
	mem.bufferAddress = (void*)sourceAddress;
	mem.size = writeSize;

	callHook(&mem);

	return mem.result;
}