#pragma once
#include <imgui.h>
#include "scanner.h"
class MainUI {
public:
	MainUI();
	void draw(Scanner& scanner, Process& proc);

private:
	inline static const char* types[] = { "1 Byte (char)", "2 Bytes (short)", "4 bytes (int)", "8 bytes (long)", "Float", "Double" };
	inline static int selectedType = 2;
	inline static double inputVal = 0.0;
};