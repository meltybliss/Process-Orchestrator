#pragma once
#include <vector>
#include <cstdint>

class MemoryPool {
public:
	uintptr_t* targetBasePtr;//senntou
	size_t offset;//tugino kakikomiiti.nannkome ka
	size_t capacity;//gennzaino youryou

	MemoryPool(size_t initialSize) {
		capacity = initialSize;

		targetBasePtr = (uintptr_t*)malloc(sizeof(uintptr_t) * capacity);
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

	uintptr_t get(size_t index) const {
		if (index < offset) {
			return targetBasePtr[index];//or it also can be *(targetBasePtr + index)
		}

		return 0;
	}

	uintptr_t* getPtr (size_t index) const {
		if (index < offset) {
			return targetBasePtr + index;//or it also can be &targetBasePtr[index];
		}

		return nullptr;
	}

	void write(uintptr_t target) {
		if (offset >= capacity) {
			grow();
		}

		//型のサイズ × 数値分だけジャンプ.だからoffsetが1でも8子分進む
		*(targetBasePtr + offset) = target;//targetBasePtr[offset] = target mo kanou.

		offset += 1;

	}

	void grow() {
		size_t newCap = this->capacity * 2;
		uintptr_t* tmp = (uintptr_t*)realloc(targetBasePtr, sizeof(uintptr_t) * newCap);

		if (tmp != nullptr) {
			targetBasePtr = tmp;
			capacity = newCap;
		}
	}


};