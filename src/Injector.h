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