#include "Process.h"

DWORD Process::GetPidByName(const char* processName)
{
    DWORD pid = 0;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (_stricmp(pe32.szExeFile, processName) == 0) {
                    pid = pe32.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }

    }


    CloseHandle(hSnapshot);

    return pid;
}

bool Process::Attach(const char* processName)
{
    DWORD pid = GetPidByName(processName);

    this->hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    m_pid = pid;
    m_processName = processName;


    return (this->hProcess != NULL);
}

void Process::Detach() {
    if (hProcess) {
        CloseHandle(hProcess);
        hProcess = NULL;
    }

    m_pid = 0;
}

bool Process::IsAlive() const {
    if (!hProcess) return false;

    DWORD code = 0;
    if (!GetExitCodeProcess(hProcess, &code)) return false;

    return code == STILL_ACTIVE;

}
