#pragma once
#include <vector>
#include <windows.h>
#include "Process.h"

class ManualInjector {
public:
	ManualInjector() = default;

	bool InjectAndExecute(Process& proc, const std::vector<unsigned char>& shellCode);
private:
	void ReportError(const char* msg);
};