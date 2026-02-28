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

				//àÍãCÇ…bufferÇ…baseAddressÇ©ÇÁÇªÇÃregionÇÃèIÇÌÇËÇ‹Ç≈ì«Ç›çûÇﬁ
				if (ReadProcessMemory(proc.hProcess, mbi.BaseAddress, buffer.data(), mbi.RegionSize, &bytesRead)) {
					for (size_t i = 0; i <= bytesRead - sizeof(T); i += sizeof(T)) {
						T value = *(T*)&buffer[i];

						if (value == targetValue) {
							uintptr_t foundAddr = (uintptr_t)mbi.BaseAddress + i;

							pool.write(foundAddr);
						}
					} 
				}


			addr = (uintptr_t)mbi.BaseAddress + mbi.RegionSize;//éüÇÃÉÅÉÇÉäÇÃâÚÇ÷à⁄ìÆ

		}




	};

};