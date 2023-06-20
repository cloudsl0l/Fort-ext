#pragma once
#pragma warning (disable : 6387 6289)
#include <cstddef>
#include <cstdint>
#include <map>

#define _WINSOCKAPI_ // don't include winsock from windows.h or else, errors lol
#include <Windows.h>
#include <TlHelp32.h>
#include <memory>
#include <string_view>
#include <cstdint>
#include <vector>
#pragma comment(lib, "PSKDM.lib")

extern std::uint32_t processId;
extern uintptr_t baseAddress;
extern ULONG64 modSize;

namespace mapper
{
	enum class mapper_error
	{
		error_success,			// everything is good!
		image_invalid,			// the driver your trying to map is invalid (are you importing things that arent in ntoskrnl?)
		load_error,			// unable to load signed driver into the kernel (are you running as admin?)
		unload_error,			// unable to unload signed driver from kernel (are all handles to this driver closes?)
		piddb_fail,			// piddb cache clearing failed... (are you using this code below windows 10?)
		init_failed,			// setting up library dependancies failed!
		failed_to_create_proc,		// was unable to create a new process to inject driver into! (RuntimeBroker.exe)
		set_mgr_failure,		// unable to stop working set manager thread... this thread can cause issues with PTM...
		zombie_process_failed
	};

	/// <summary>
	/// map a driver only into your current process...
	/// </summary>
	/// <param name="drv_image">base address of driver buffer</param>
	/// <param name="image_size">size of the driver buffer</param>
	/// <param name="entry_data">data to be sent to the entry point of the driver...</param>
	/// <returns>status of the driver being mapped, and base address of the driver...</returns>
	auto map_driver(std::uint32_t pid, std::uint8_t* drv_image, std::size_t image_size, void** entry_data) -> std::pair<mapper_error, void*>;
}

typedef struct _MEMSTRUCT
{
	// Process
	ULONG pid;
	const char* moduleName;
	ULONG64 baseAddress;
	ULONG64 modSize;

	UINT_PTR address;
	//const char* error;
	int result;

	// Get base
	BOOLEAN getBaseAdress;

	// Get size
	BOOLEAN getModSize;

	// Write
	BOOLEAN write;
	void* bufferAddress;
	ULONGLONG size;

	// Read
	BOOLEAN read;
	void* output;

} MEMSTRUCT, * PMEMSTRUCT;

template<typename ... Arg>
uint64_t callHook(const Arg ... args);

// https://www.unknowncheats.me/forum/2534454-post3.html
struct HandleDisposer
{
	using pointer = HANDLE;
	void operator()(HANDLE handle) const
	{
		if (handle != 0 || handle != INVALID_HANDLE_VALUE)
			CloseHandle(handle);
	}
};

using unique_handle = std::unique_ptr<HANDLE, HandleDisposer>;
static std::uint32_t GetProcess(std::wstring_view process_name)
{
	PROCESSENTRY32W processentry;
	const unique_handle snapshot_handle(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));

	if (snapshot_handle.get() == INVALID_HANDLE_VALUE)
		return 0;

	processentry.dwSize = sizeof(PROCESSENTRY32W);

	while (Process32NextW(snapshot_handle.get(), &processentry) == 1)
	{
		if (process_name.compare(processentry.szExeFile) == 0)
			return processentry.th32ProcessID;
	}

	return 0;
}

ULONG64 GetModuleBaseAddress(const char* moduleName);

ULONG64 GetModuleSize(const char* moduleName);

template <class T>
T Read(UINT_PTR readAddress)
{
	T result{};
	MEMSTRUCT mem = { 0 };
	mem.pid = processId;
	mem.size = sizeof(T);
	mem.address = readAddress;
	mem.read = 1;
	mem.write = 0;
	mem.getBaseAdress = 0;
	mem.output = &result;
	callHook(&mem);

	return result;
}

bool WriteMem(UINT_PTR writeAddress, UINT_PTR sourceAddress, SIZE_T writeSize);

template <typename S>
bool Write(UINT_PTR writeAddress, const S& value)
{
	return WriteMem(writeAddress, (UINT_PTR)&value, sizeof(S));
}
