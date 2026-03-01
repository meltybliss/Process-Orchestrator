#pragma once
#include <vector>
#include <windows.h>
#include <fstream>
#include <filesystem>
#include "Process.h"

class ManualInjector {
public:
	ManualInjector() = default;

	bool InjectAndExecute(Process& proc, const std::vector<unsigned char>& shellCode);
	bool ManualMap(Process& proc, const char* dllPath);
private:
	void ReportError(const char* msg);
};