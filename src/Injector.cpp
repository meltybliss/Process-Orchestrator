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

bool ManualInjector::ManualMap(Process& proc, const char* dllPath)
{
	// 1. DLLファイルをバイナリとして読み込む
	std::ifstream file(dllPath, std::ios::binary | std::ios::ate);//ate mode
	if (file.fail()) return false;

	size_t fileSize = file.tellg();
	std::vector<uint8_t> rawData(fileSize);
	file.seekg(0, std::ios::beg);
	file.read((char*)rawData.data(), fileSize);
	file.close();

	// 2. PEヘッダーの確認（これが正しいDLLファイルかチェック）
	auto* dosHeader = (PIMAGE_DOS_HEADER)rawData.data();
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return false;

	auto* ntHeader = (PIMAGE_NT_HEADERS)(rawData.data() + dosHeader->e_lfanew);
	if (ntHeader->Signature != IMAGE_NT_SIGNATURE) return false;

	// 3. ターゲットプロセス内にメモリを確保
	// DLLが要求するサイズ分（SizeOfImage）を確保する
	void* targetBase = VirtualAllocEx(proc.hProcess, nullptr,
		ntHeader->OptionalHeader.SizeOfImage, 
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if (!targetBase) return false;

	// --- ここから先の処理（再配置やインポート解決）はさらに複雑になります ---

	return true;

}

void ManualInjector::ReportError(const char* msg)
{
	std::cerr << "[Injector Error] " << msg << " | Code: " << GetLastError() << "\n";
}
