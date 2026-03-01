#pragma once
#include <vector>
#include "MemoryPool.h"
#include "Process.h"
#include <iostream>
class Scanner {
public:
	Scanner() : pool(1024 * 1024) {};//1mb

	template<typename T>
	static inline void Store8(unsigned char dst[8] , const T& v) {
		static_assert(sizeof(T) <= 8);
		std::memset(dst, 0, 8);
		std::memcpy(dst, &v, sizeof(T));
	
	}

	template<typename T>
	static inline T Load8(const unsigned char src[8]) {
		static_assert(sizeof(T) <= 8);
		T v{};
		std::memcpy(&v, src, sizeof(T));
		return v;
	}

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
						//T value = *(T*)&buffer[i];//1バイトしか指せないはずの場所（char）を、一度アドレス（ポインタ）に変換することで、そこからNバイト分（T）まで視界を広げる
						T value;
						std::memcpy(&value, &buffer[i], sizeof(T));//T value = *(T*)&buffer[i]と同じ

						if (value == targetVal) {
							ScanResult result;
							result.addr = (uintptr_t)mbi.BaseAddress + i;

							Store8(result.prevValue, value);
							Store8(result.lastValue, value);

							//*(T*)result.prevValue = value;
							//*(T*)result.lastValue = value;

							pool.write(result);
						}
					}
				}
			}

			addr = (uintptr_t)mbi.BaseAddress + mbi.RegionSize;//次のメモリの塊へ移動

		}

	}


	template<typename T>
	void nextSearch(Process& proc, T targetVal) {
		ScanResult* readPtr = pool.targetBasePtr;

		ScanResult* writePtr = readPtr;

		ScanResult* endPtr = pool.targetBasePtr + pool.offset;//終端ポインタ

		while (readPtr < endPtr) {
			T curVal;

			if (proc.Read(readPtr->addr, &curVal)) {
				if (curVal == targetVal) {

					std::memcpy(readPtr->prevValue, readPtr->lastValue, 8);
					Store8(readPtr->lastValue, curVal);
					//*(T*)readPtr->prevValue = *(T*)readPtr->lastValue;
					//*(T*)readPtr->lastValue = curVal;
					*writePtr = *readPtr;
					writePtr++;
				}
			}

			readPtr++;

		}

		pool.offset = (size_t)(writePtr - pool.targetBasePtr);//ポインタ同士を引き算すると、バイト数ではなく**「その型（ScanResult）が何個分入るか
	}

	size_t getResults() {
		return pool.offset;
	}

	ScanResult* getTaregetPtr(size_t index) const {
		return pool.getPtr(index);
	}

private:
	
	MemoryPool pool;


};