#include "mainUI.h"



MainUI::MainUI()
{
	
}

void MainUI::draw(Scanner& scanner, Process& proc) {

	ImGui::Begin("Main UI");

	ImGui::Text("Search Type:");
	ImGui::Separator();

	ImGui::SetNextItemWidth(150);
	ImGui::Combo("##TypeCombo", &selectedType, types, IM_ARRAYSIZE(types));

	ImGui::SetNextItemWidth(200);
	
	if (selectedType == 4) {
		float tempFloat = (float)inputVal;
		if (ImGui::InputFloat("Value", &tempFloat)) {
			inputVal = (double)tempFloat;
		}
	}
	else if (selectedType == 5) {
	
		ImGui::InputDouble("value", &inputVal);
	}
	else {
		int tempInt = (int)inputVal;
		if (ImGui::InputInt("value", &tempInt)) {
			inputVal = (int)tempInt;
		}
	}

	if (ImGui::Button("First Search", ImVec2(100, 30))) {
		switch (selectedType) {
			case 0: scanner.firstSearch(proc, (int8_t)inputVal); break;
			case 1: scanner.firstSearch(proc, (int16_t)inputVal); break;
			case 2: scanner.firstSearch(proc, (int32_t)inputVal); break;
			case 3: scanner.firstSearch(proc, (int64_t)inputVal); break;
			case 4: scanner.firstSearch(proc, (float)inputVal); break;
			case 5: scanner.firstSearch(proc, inputVal); break;
		}
	}

	ImGui::End();
}