#pragma once
#include <vector>
#include <cstdint>

struct ScanResult {
	uintptr_t addr;
	unsigned char prevValue[8];
	unsigned char lastValue[8];//8bytes
};

class MemoryPool {
public:
	ScanResult* targetBasePtr;//senntou
	size_t offset;//tugino kakikomiiti.何個入ってるか（件数）
	size_t capacity;//gennzaino youryou

	MemoryPool(size_t initialSize) {
		capacity = initialSize;

		targetBasePtr = (ScanResult*)malloc(sizeof(ScanResult) * capacity);
		if (targetBasePtr == nullptr) {
			capacity = 0;
		}

		offset = 0;

	};

	~MemoryPool() {
		if (targetBasePtr != nullptr) {
			free(targetBasePtr);
		}
	}


	ScanResult* getPtr (size_t index) const {
		if (index < offset) {
			return targetBasePtr + index;//or it also can be &targetBasePtr[index];
		}

		return nullptr;
	}

	void write(ScanResult result) {
		if (offset >= capacity) {
			grow();
		}

		//型のサイズ × 数値分だけジャンプ.だからoffsetが1でも8子分進む
		*(targetBasePtr + offset) = result;//targetBasePtr[offset] =  mo kanou.

		offset++;

	}

	void grow() {
		size_t newCap = this->capacity * 2;
		ScanResult* tmp = (ScanResult*)realloc(targetBasePtr, sizeof(ScanResult) * newCap);

		if (tmp != nullptr) {
			targetBasePtr = tmp;
			capacity = newCap;
		}
	}


};