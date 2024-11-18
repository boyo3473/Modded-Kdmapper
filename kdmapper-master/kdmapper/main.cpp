#ifndef KDLIBMODE

#include <Windows.h>
#include <string>
#include <vector>
#include <filesystem>
#include <TlHelp32.h> 
#include <iostream>   

#include "kdmapper.hpp"

HANDLE iqvw64e_device_handle;


const std::vector<uint8_t> driver_data = {

	// paste your driver in hex here  (use HxD and explort as C)
};

LONG WINAPI SimplestCrashHandler(EXCEPTION_POINTERS* ExceptionInfo)
{
	if (ExceptionInfo && ExceptionInfo->ExceptionRecord)
		
		return EXCEPTION_EXECUTE_HANDLER;
	else
		return EXCEPTION_EXECUTE_HANDLER;
}



DWORD getParentProcess() {
	HANDLE hSnapshot;
	PROCESSENTRY32 pe32;
	DWORD ppid = 0, pid = GetCurrentProcessId();

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	__try {
		if (hSnapshot == INVALID_HANDLE_VALUE) __leave;

		ZeroMemory(&pe32, sizeof(pe32));
		pe32.dwSize = sizeof(pe32);
		if (!Process32First(hSnapshot, &pe32)) __leave;

		do {
			if (pe32.th32ProcessID == pid) {
				ppid = pe32.th32ParentProcessID;
				break;
			}
		} while (Process32Next(hSnapshot, &pe32));
	}
	__finally {
		if (hSnapshot != INVALID_HANDLE_VALUE) CloseHandle(hSnapshot);
	}
	return ppid;
}

int paramExists(const int argc, wchar_t** argv, const wchar_t* param) {
	size_t plen = wcslen(param);
	for (int i = 1; i < argc; i++) {
		if (wcslen(argv[i]) == plen + 1ull && _wcsicmp(&argv[i][1], param) == 0 && argv[i][0] == '/') {
			return i;
		}
		else if (wcslen(argv[i]) == plen + 2ull && _wcsicmp(&argv[i][2], param) == 0 && argv[i][0] == '-' && argv[i][1] == '-') {
			return i;
		}
	}
	return -1;
}

bool callbackExample(ULONG64* param1, ULONG64* param2, ULONG64 allocationPtr, ULONG64 allocationSize) {
	UNREFERENCED_PARAMETER(param1);
	UNREFERENCED_PARAMETER(param2);
	UNREFERENCED_PARAMETER(allocationPtr);
	UNREFERENCED_PARAMETER(allocationSize);

	return true;
}

void PauseIfParentIsExplorer() {
	DWORD explorerPid = 0;
	GetWindowThreadProcessId(GetShellWindow(), &explorerPid);
	DWORD parentPid = getParentProcess();
	if (parentPid == explorerPid) {
		
		std::cin.get();
	}
}

int wmain(const int argc, wchar_t** argv) {
	SetUnhandledExceptionFilter(SimplestCrashHandler);

	bool free = (paramExists(argc, argv, L"free") > 0);
	bool indPagesMode = (paramExists(argc, argv, L"indPages") > 0);
	bool passAllocationPtr = (paramExists(argc, argv, L"PassAllocationPtr") > 0);

	
	iqvw64e_device_handle = intel_driver::Load();

	if (iqvw64e_device_handle == INVALID_HANDLE_VALUE) {
		PauseIfParentIsExplorer();
		return -1;
	}

	kdmapper::AllocationMode mode = kdmapper::AllocationMode::AllocatePool;

	if (indPagesMode) {
		mode = kdmapper::AllocationMode::AllocateIndependentPages;
	}

	NTSTATUS exitCode = 0;
	if (!kdmapper::MapDriver(
		iqvw64e_device_handle,
		const_cast<BYTE*>(reinterpret_cast<const BYTE*>(driver_data.data())),
		0,
		0,
		free,
		true,
		mode,
		passAllocationPtr,
		callbackExample,
		&exitCode)) {
		intel_driver::Unload(iqvw64e_device_handle);
		PauseIfParentIsExplorer();
		return -1;
	}

	if (!intel_driver::Unload(iqvw64e_device_handle)) {
		PauseIfParentIsExplorer();
	}

	std::cout << "Driver Loaded!" << std::endl;

	system("pause");

	return 0;
}

#endif