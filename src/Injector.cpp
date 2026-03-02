#include "Injector.h"
#include <iostream>


void __stdcall ShellCode(MANUAL_MAPPING_DATA* pData);


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
	// 4. セクションのコピー
	// NTヘッダーのすぐ後ろにある「セクションヘッダー（目次）」の先頭を取得
	PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(ntHeader);

	for (UINT i = 0; i < ntHeader->FileHeader.NumberOfSections; ++i) {
		if (pSectionHeader[i].SizeOfRawData > 0) {
			// コピー先：確保したメモリ(targetBase) + 仮想的なズレ(VirtualAddress)
			void* dest = (uint8_t*)targetBase + pSectionHeader[i].VirtualAddress;
			// コピー元：読み込んだデータ(rawData) + ファイル上のズレ(PointerToRawData)
			void* src = rawData.data() + pSectionHeader[i].PointerToRawData;
			// 実際にターゲットプロセスのメモリへ書き込む
			if (!WriteProcessMemory(proc.hProcess, dest, src, pSectionHeader[i].SizeOfRawData, nullptr)) {
				VirtualFreeEx(proc.hProcess, targetBase, 0, MEM_RELEASE);
				return false;
			}
		}
	}
	// --- ステップ5: 指示書の準備 ---
	MANUAL_MAPPING_DATA data{ 0 };
	data.pLoadLibraryA = (f_LoadLibraryA)LoadLibraryA;
	data.pGetProcAddress = (f_GetProcAddress)GetProcAddress;
	data.pbase = targetBase;
	data.fdwReasonParam = DLL_PROCESS_ATTACH;

	// ターゲットプロセス内に構造体用のメモリを確保
	void* pMappingDataAlloc = VirtualAllocEx(proc.hProcess, nullptr, sizeof(MANUAL_MAPPING_DATA), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!pMappingDataAlloc) return false;

	// 指示書を書き込む
	WriteProcessMemory(proc.hProcess, pMappingDataAlloc, &data, sizeof(MANUAL_MAPPING_DATA), nullptr);

	// --- ステップ6: シェルコードの転送と実行 ---
	// シェルコード用のメモリを確保（実行権限 PAGE_EXECUTE_READWRITE が必須）
	void* pShellCodeAlloc = VirtualAllocEx(proc.hProcess, nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pShellCodeAlloc) return false;

	//定義したShellcode 関数をターゲットに書き込む. Shellcode関数をそのままコピーして送り込む
	WriteProcessMemory(proc.hProcess, pShellCodeAlloc, ShellCode, 0x1000, nullptr);

	// 遠隔スレッドを作成して、シェルコードを起動！
	// pMappingDataAlloc（指示書の住所）を引数として渡す
	HANDLE hThread = CreateRemoteThread(proc.hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)pShellCodeAlloc, pMappingDataAlloc, 0, nullptr);
	if (!hThread) return false;

	HINSTANCE hCheck = NULL;
	while (!hCheck) {
		MANUAL_MAPPING_DATA data_checked{ 0 };
		ReadProcessMemory(proc.hProcess, pMappingDataAlloc, &data_checked, sizeof(data_checked), nullptr);
		hCheck = data_checked.hMod;
		Sleep(10);//cpu wo 10 byou yasumeru
	}

	printf("DLL Mapping Successful! At: %p\n", hCheck);

	return true;

}

void ManualInjector::ReportError(const char* msg)
{
	std::cerr << "[Injector Error] " << msg << " | Code: " << GetLastError() << "\n";
}

#pragma runtime_checks("", off)
#pragma optimize("", off)

void __stdcall ShellCode(MANUAL_MAPPING_DATA* pData) {
	if (!pData) return;

	// 指示書から「ベースのアドレス」を取り出す
	BYTE* pBase = (BYTE*)pData->pbase;
	auto* pOpt = &((PIMAGE_NT_HEADERS)(pBase + ((PIMAGE_DOS_HEADER)pBase)->e_lfanew))->OptionalHeader;

	// 指示書に書かれた「住所」を使って、関数を使えるようにする
	auto _LoadLibraryA = pData->pLoadLibraryA;
	auto _GetProcAddress = pData->pGetProcAddress;
	auto _DllMain = (f_DLL_ENTRY_POINT)(pBase + pOpt->AddressOfEntryPoint);

	// 【ここがメイン】
	// 1. 再配置 (Relocation) の計算
	// 2. インポートの解決
	// 3. DllMain の実行
	
	// --- 1. 再配置 (Relocation) の計算 --
	// 「理想の住所」と「実際の住所」の差（Delta）を計算
	// これがプラスなら、その分だけ各アドレスに足してあげる必要がある
	BYTE* LocationDelta = pBase - pOpt->ImageBase;

	if (LocationDelta != 0) {//住所がズレている場合のみ実行
		// 再配置テーブルの場所を特定
		auto* pRelocData = (IMAGE_BASE_RELOCATION*)(pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);

		while (pRelocData->VirtualAddress) {
			// このブロック内の修正箇所の数
			UINT AmountOfEntries = (pRelocData->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			WORD* pRelativeInfo = (WORD*)(pRelocData + 1);// 修正情報のリスト

			for (UINT i = 0; i < AmountOfEntries; ++i) {
				// 上位4ビットがタイプ、下位12ビットがオフセット
				if ((pRelativeInfo[i] >> 12) == IMAGE_REL_BASED_HIGHLOW || (pRelativeInfo[i] >> 12) == IMAGE_REL_BASED_DIR64) {
					// 修正すべき場所のアドレスを計算
					uintptr_t* pPatch = (uintptr_t*)(pBase + pRelocData->VirtualAddress + (pRelativeInfo[i] & 0xFFF));
					// 差分（Delta）を足して、住所を「今」の場所に更新！
					*pPatch += (uintptr_t)LocationDelta;
				}
			}
			// 次の修正ブロックへ移動
			pRelocData = (IMAGE_BASE_RELOCATION*)((BYTE*)pRelocData + pRelocData->SizeOfBlock);
		}
	}

	// --- 2. インポートの解決 (Import Resolution) ---

	// インポートディレクトリ（「どのDLLのどの関数を使うか」のリスト）を探す
	if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
		auto* pImportDescr = (IMAGE_IMPORT_DESCRIPTOR*)(pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		while (pImportDescr->Name) {
			// 1. 必要なDLL（例：user32.dll）の名前を取得して読み込む
			char* szMod = (char*)(pBase + pImportDescr->Name);
			HINSTANCE hDll = _LoadLibraryA(szMod);

			// 2. 関数の住所リスト（IAT）の場所を特定
			ULONG_PTR* pThunkRef = (ULONG_PTR*)(pBase + pImportDescr->OriginalFirstThunk);
			ULONG_PTR* pFuncRef = (ULONG_PTR*)(pBase + pImportDescr->FirstThunk);

			if (!pImportDescr->OriginalFirstThunk)
				pThunkRef = pFuncRef;

			// 3. そのDLLから必要な関数の住所を一つずつ取ってくる
			for (; *pThunkRef; ++pThunkRef, ++pFuncRef) {
				if (IMAGE_SNAP_BY_ORDINAL(*pThunkRef)) {
					// 名前ではなく「番号（Ordinal）」で呼ぶ場合
					*pFuncRef = (ULONG_PTR)_GetProcAddress(hDll, (char*)(*pThunkRef & 0xFFFF));
				}
				else {
					// 名前（例："MessageBoxA"）で呼ぶ場合
					auto* pImport = (IMAGE_IMPORT_BY_NAME*)(pBase + (*pThunkRef));
					*pFuncRef = (ULONG_PTR)_GetProcAddress(hDll, pImport->Name);
				}
			}
			pImportDescr++; // 次のDLLのリストへ
		}
	}

	// --- 3. DllMain の実行 ---

	_DllMain(pBase, pData->fdwReasonParam, nullptr);
	// 最後に、成功した証としてベースアドレスを指示書に書き残しておく
	pData->hMod = (HINSTANCE)pBase;
}