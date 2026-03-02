#pragma once
#include <vector>
#include <windows.h>
#include <fstream>
#include <filesystem>
#include "Process.h"

//シェルコードが使うAPIの住所用
//シェルコードの中で LoadLibraryA などを呼び出すために、関数の型を定義
using f_LoadLibraryA = HINSTANCE(WINAPI*)(const char* lpLibFileName);
using f_GetProcAddress = UINT_PTR(WINAPI*)(HMODULE hModule, const char* lpProcName);
using f_DLL_ENTRY_POINT = BOOL(WINAPI*)(void* hDLL, DWORD dwReason, void* pReserved);

struct MANUAL_MAPPING_DATA {
	f_LoadLibraryA pLoadLibraryA;
	f_GetProcAddress pGetProcAddress;
	void* pbase;// DLLが置かれた場所 (targetBase)
	DWORD fdwReasonParam;// DLL_PROCESS_ATTACH 等のフラグ
	HINSTANCE hMod;//成功したらここに DLL の住所が入る
};

class ManualInjector {
public:
	ManualInjector() = default;

	bool InjectAndExecute(Process& proc, const std::vector<unsigned char>& shellCode);
	bool ManualMap(Process& proc, const char* dllPath);
private:
	void ReportError(const char* msg);
};