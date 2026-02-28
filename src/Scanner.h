#pragma once
#include <vector>
#include "MemoryPool.h"
#include "Process.h"

class Scanner {
public:
	Scanner()


private:
	
	std::vector<uintptr_t*> searchResult;
	MemoryPool pool;

	template<typename T>
	void firstSearch(Process& proc, T targetVal) {
		pool.offset = 0;

		MEMORY_BASIC_INFORMATION mbi;
		uintptr_t addr = 0;

		std::vector<char> buffer;

		while (VirtualQueryEx(proc.hProcess, (LPCVOID)addr, &mbi, sizeof(mbi))) {
			if (mbi.State == MEM_COMMIT && mbi.Protect == PAGE_READWRITE) {

				buffer.resize(mbi.RegionSize);
				SIZE_T bytesRead = 0;

				//一気にbufferにbaseAddressからそのregionの終わりまで読み込む
				if (ReadProcessMemory(proc.hProcess, mbi.BaseAddress, buffer.data(), mbi.RegionSize, &bytesRead)) {
					for (size_t i = 0; i <= bytesRead - sizeof(T); i += sizeof(T)) {
						T value = *(T*)&buffer[i];//1バイトしか指せないはずの場所（char）を、一度アドレス（ポインタ）に変換することで、そこからNバイト分（T）まで視界を広げる

						if (value == targetValue) {
							ScanResult result;
							result.addr = (uintptr_t)mbi.BaseAddress + i;
							*(T*)result.lastValue = value;

							pool.write(result);
						}
					}
				}
			}

			addr = (uintptr_t)mbi.BaseAddress + mbi.RegionSize;//次のメモリの塊へ移動

		}

	}


	template<typename T>
	void nextSearch(Process & proc, T targetVal) {
		ScanResult* readPtr = pool.targetBasePtr;

		ScanResult* writePtr = readPtr;

		ScanResult* endPtr = pool.targetBasePtr + pool.offset;//終端ポインタ

		while (readPtr < endPtr) {
			T curVal;

			if (proc.Read(readPtr->addr, curVal)) {
				if (curVal == targetVal) {
					*(T*)readPtr->lastValue = curVal;
					*writePtr = *readPtr;
					writePtr++;
				}
			}

			readPtr++;

		}

		pool.offset = (size_t)(writePtr - pool.targetBasePtr);//ポインタ同士を引き算すると、バイト数ではなく**「その型（ScanResult）が何個分入るか
	}

};