#pragma once
#include <Windows.h>
#include <tlhelp32.h>
#include <string_view>
//for wrapping Windows api
class Process {
public:
	HANDLE hProcess;

	DWORD GetPidByName(const char* processName);
	bool Attach(const char* processName);

	template<typename T> 
	bool Read(uintptr_t address, T* buffer) {
		return ReadProcessMemory(this->hProcess, (LPCVOID)address, buffer, sizeof(T), NULL);
	}

};