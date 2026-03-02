#pragma once
#include <vector>
#include <windows.h>
#include <fstream>
#include <filesystem>
#include "Process.h"

//シェルコードが使うAPIの住所用
//シェルコードの中で LoadLibraryA などを呼び出すために、関数の型を定義
using f_LoadLibraryA = HINSTANCE(WINAPI*)(LPCSTR lpLibFileName);
using f_GetProcAddress = FARPROC(WINAPI*)(HMODULE hModule, LPCSTR lpProcName);
using f_DLL_ENTRY_POINT = BOOL(WINAPI*)(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

#ifdef _WIN64
using f_RtlAddFunctionTable = BOOL(WINAPI*)(PRUNTIME_FUNCTION FunctionTable, DWORD EntryCount, DWORD64 BaseAddress);
#endif

#pragma pack(push, 8)
struct MANUAL_MAPPING_DATA {
    f_LoadLibraryA pLoadLibraryA;
    f_GetProcAddress pGetProcAddress;
#ifdef _WIN64
    f_RtlAddFunctionTable pRtlAddFunctionTable;
#endif
    BYTE* pbase;
    DWORD           fdwReasonParam;
    LPVOID          reservedParam;
    BOOL            SEHSupport;
    HINSTANCE       hMod;
    uintptr_t originalRip;
};
#pragma pack(pop)

class ManualInjector {
public:
	ManualInjector() = default;

	bool InjectAndExecute(Process& proc, const std::vector<unsigned char>& shellCode);
	bool ManualMap(Process& proc, const char* dllPath);
private:
	void ReportError(const char* msg);
};