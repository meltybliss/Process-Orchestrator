#include "mainUI.h"

static void ApplyProTheme()
{
    ImGuiStyle& s = ImGui::GetStyle();
    ImGui::StyleColorsDark();

    // 丸みと余白（“Windowsツール”っぽさ）
    s.WindowRounding = 10.0f;
    s.ChildRounding = 10.0f;
    s.FrameRounding = 8.0f;
    s.PopupRounding = 10.0f;
    s.ScrollbarRounding = 12.0f;
    s.GrabRounding = 8.0f;
    s.TabRounding = 8.0f;

    s.WindowPadding = ImVec2(10, 10);
    s.FramePadding = ImVec2(10, 6);
    s.ItemSpacing = ImVec2(10, 8);
    s.ItemInnerSpacing = ImVec2(8, 6);

    s.WindowBorderSize = 1.0f;
    s.FrameBorderSize = 0.0f;
    s.PopupBorderSize = 1.0f;
    s.TabBorderSize = 0.0f;

    // “ギラつきすぎない”アクセント色（シアン寄り）
    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.08f, 0.10f, 1.00f);
    c[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.07f, 0.09f, 1.00f);
    c[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.09f, 0.11f, 1.00f);

    c[ImGuiCol_Border] = ImVec4(0.18f, 0.20f, 0.26f, 1.00f);
    c[ImGuiCol_Separator] = c[ImGuiCol_Border];

    c[ImGuiCol_Text] = ImVec4(0.92f, 0.94f, 0.98f, 1.00f);
    c[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.60f, 0.70f, 1.00f);

    c[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.13f, 0.16f, 1.00f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.18f, 0.22f, 1.00f);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.20f, 0.26f, 1.00f);

    // アクセント（ここが“青”から脱却する場所）
    ImVec4 accent = ImVec4(0.20f, 0.85f, 0.90f, 1.00f); // シアン
    ImVec4 accentHover = ImVec4(0.25f, 0.92f, 0.96f, 1.00f);
    ImVec4 accentActive = ImVec4(0.15f, 0.75f, 0.80f, 1.00f);

    c[ImGuiCol_Button] = ImVec4(0.14f, 0.16f, 0.20f, 1.00f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.18f, 0.20f, 0.26f, 1.00f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.30f, 1.00f);

    c[ImGuiCol_CheckMark] = accent;
    c[ImGuiCol_SliderGrab] = accent;
    c[ImGuiCol_SliderGrabActive] = accentHover;
    c[ImGuiCol_ResizeGrip] = accentActive;
    c[ImGuiCol_ResizeGripHovered] = accent;
    c[ImGuiCol_ResizeGripActive] = accentHover;

    c[ImGuiCol_Header] = ImVec4(accent.x, accent.y, accent.z, 0.18f);
    c[ImGuiCol_HeaderHovered] = ImVec4(accent.x, accent.y, accent.z, 0.28f);
    c[ImGuiCol_HeaderActive] = ImVec4(accent.x, accent.y, accent.z, 0.38f);

    c[ImGuiCol_Tab] = ImVec4(0.11f, 0.12f, 0.15f, 1.00f);
    c[ImGuiCol_TabHovered] = ImVec4(accent.x, accent.y, accent.z, 0.35f);
    c[ImGuiCol_TabActive] = ImVec4(0.13f, 0.15f, 0.19f, 1.00f);

    c[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.07f, 0.09f, 1.00f);
    c[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.08f, 0.10f, 1.00f);

    c[ImGuiCol_ScrollbarBg] = ImVec4(0.06f, 0.07f, 0.09f, 1.00f);
    c[ImGuiCol_ScrollbarGrab] = ImVec4(0.18f, 0.20f, 0.26f, 1.00f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.22f, 0.24f, 0.32f, 1.00f);
    c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.26f, 0.28f, 0.38f, 1.00f);

    // テーブルの見た目（結果一覧が“ツール感”出る）
    c[ImGuiCol_TableHeaderBg] = ImVec4(0.10f, 0.11f, 0.14f, 1.00f);
    c[ImGuiCol_TableBorderStrong] = ImVec4(0.18f, 0.20f, 0.26f, 1.00f);
    c[ImGuiCol_TableBorderLight] = ImVec4(0.14f, 0.16f, 0.20f, 1.00f);
    c[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    c[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);
}

MainUI::MainUI()
{
	
}
void MainUI::draw(Scanner& scanner, Process& proc)
{
    static bool freezeResults = false;
    static char addrFilter[64] = "";      // "0x7FF..." とか "7FF" でもOK
    static int shownLimit = 5000;
    static int selectedRow = -1;
    static double lastScanMs = 0.0;

    static bool AppliedTheme = false;
    if (!AppliedTheme) {
        ApplyProTheme();
        AppliedTheme = true;
    }

    // --- window fullscreen ---
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(vp->WorkSize, ImGuiCond_Always);

    ImGuiWindowFlags wf =
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Main UI", nullptr, wf);

    // --- top bar ---
    {
        const size_t rc = scanner.getResults();
        ImGui::Text("PID: %d", proc.pid());
        ImGui::SameLine();
        ImGui::Text("| Results: %zu", rc);
        ImGui::SameLine();
        ImGui::Text("| Last: %.2f ms", lastScanMs);
        ImGui::SameLine();
        ImGui::Checkbox("Freeze", &freezeResults);
    }
    ImGui::Separator();

    // --- remaining region split ---
    ImVec2 content = ImGui::GetContentRegionAvail();
    float leftW = 320.0f;



    // ====== 左右ペイン ======
    ImGui::BeginChild("LeftPane", ImVec2(leftW, content.y), true);
    {
        static char procName[128] = "game.exe";
        static bool attached = false;

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 0.85f, 0.90f, 1.00f));
        ImGui::TextUnformatted("Process");
        ImGui::PopStyleColor();
        ImGui::Separator();

        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##proc", procName, IM_ARRAYSIZE(procName));
        ImGui::SameLine();
        ImGui::TextDisabled("exe name");

        if (!proc.IsAttached()) {
            if (ImGui::Button("Attach", ImVec2(-1, 28))) {
                attached = proc.Attach(procName);
            }
        }
        else {
            if (ImGui::Button("Detach", ImVec2(-1, 28))) {
                proc.Detach();
            }
        }

        ImGui::Text("Status: %s", proc.IsAttached() ? (proc.IsAlive() ? "Attached" : "Dead") : "Not attached");
        ImGui::Text("PID: %lu", (unsigned long)proc.pid());
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 0.85f, 0.90f, 1.00f));
        ImGui::Text("Search");
        ImGui::PopStyleColor();
        ImGui::Separator();

        ImGui::SetNextItemWidth(-1);
        ImGui::Combo("Type", &selectedType, types, IM_ARRAYSIZE(types));

        // 入力（型に合わせて）
        if (selectedType == 4) {
            float temp = (float)inputVal;
            if (ImGui::InputFloat("Value", &temp)) inputVal = (double)temp;
        }
        else if (selectedType == 5) {
            ImGui::InputDouble("Value", &inputVal);
        }
        else {
            int temp = (int)inputVal;
            if (ImGui::InputInt("Value", &temp)) inputVal = (double)temp;
        }

        ImGui::Spacing();

        // ボタン類（横並び）
        if (ImGui::Button("First Scan", ImVec2(-1, 32))) {
            const double t0 = ImGui::GetTime();
            switch (selectedType) {
            case 0: scanner.firstSearch(proc, (int8_t)inputVal); break;
            case 1: scanner.firstSearch(proc, (int16_t)inputVal); break;
            case 2: scanner.firstSearch(proc, (int32_t)inputVal); break;
            case 3: scanner.firstSearch(proc, (int64_t)inputVal); break;
            case 4: scanner.firstSearch(proc, (float)inputVal); break;
            case 5: scanner.firstSearch(proc, (double)inputVal); break;
            }
            lastScanMs = (ImGui::GetTime() - t0) * 1000.0;
            selectedRow = -1;
        }

        if (ImGui::Button("Next Scan", ImVec2(-1, 32))) {
            const double t0 = ImGui::GetTime();
            // ここはあなたのScanner APIに合わせて呼び出しを作る
            switch (selectedType) {
            case 0: scanner.nextSearch(proc, (int8_t)inputVal); break;
            case 1: scanner.nextSearch(proc, (int16_t)inputVal); break;
            case 2: scanner.nextSearch(proc, (int32_t)inputVal); break;
            case 3: scanner.nextSearch(proc, (int64_t)inputVal); break;
            case 4: scanner.nextSearch(proc, (float)inputVal); break;
            case 5: scanner.nextSearch(proc, (double)inputVal); break;
            }
            lastScanMs = (ImGui::GetTime() - t0) * 1000.0;
        }

        /*if (ImGui::Button("Reset Results", ImVec2(-1, 28))) {
            scanner.clearResults();
            selectedRow = -1;
        }*/

        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 0.85f, 0.90f, 1.00f));
        ImGui::Text("Result View");
        ImGui::PopStyleColor();

        ImGui::Separator();

        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("Addr filter", addrFilter, IM_ARRAYSIZE(addrFilter));

        ImGui::SetNextItemWidth(-1);
        ImGui::SliderInt("Show limit", &shownLimit, 100, 50000);

    }

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("RightPane", ImVec2(0, content.y), true);
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 0.85f, 0.90f, 1.00f));
        ImGui::Text("Results");
        ImGui::PopStyleColor();
        ImGui::Separator();

        // 結果テーブル
        ImGuiTableFlags flags =
            ImGuiTableFlags_Resizable |
            ImGuiTableFlags_Reorderable |
            ImGuiTableFlags_Hideable |
            ImGuiTableFlags_Sortable |
            ImGuiTableFlags_RowBg |
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_ScrollY;


        ImVec2 avail = ImGui::GetContentRegionAvail();
        if (ImGui::BeginTable("ResultsTable", 4, flags, avail)) {
            ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_DefaultSort);
            ImGui::TableSetupColumn("Value");
            ImGui::TableSetupColumn("Prev");
            ImGui::TableSetupColumn("Delta");
            ImGui::TableHeadersRow();

            // 高速化の定番：クリップ
            ImGuiListClipper clipper;

           
            const int n = (int)scanner.getResults();

            clipper.Begin(n);
            int shown = 0;

            while (clipper.Step()) {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                    if (shown >= shownLimit) break;

                    const auto& r = scanner.getTareget((size_t)i);

                    // フィルタ（簡易：アドレスをhex文字列化して部分一致）
                    if (addrFilter[0] != '\0') {
                        char buf[32];
                        sprintf_s(buf, "0x%llX", (unsigned long long)r.addr);
                        if (strstr(buf, addrFilter) == nullptr) continue;
                    }

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);

                    // 行選択
                    bool isSelected = (selectedRow == i);
                    char addrStr[32];
                    sprintf_s(addrStr, "0x%llX", (unsigned long long)r.addr);

                    if (ImGui::Selectable(addrStr, isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
                        selectedRow = i;
                    }

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.6f", *(double*)r.lastValue);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%.6f", *(double*)r.prevValue);

                    ImGui::TableSetColumnIndex(3);
                    double v = *(double*)r.lastValue;
                    double p = *(double*)r.prevValue;
                    ImGui::Text("%.6f", v - p);

                    ++shown;
                }
            }

            ImGui::EndTable();
        }

        
    }
    ImGui::EndChild();
    ImGui::End();
}