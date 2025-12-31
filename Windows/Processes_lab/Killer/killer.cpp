#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include <string>

const DWORD MIN_PID = 4;
const DWORD MAX_PID = 0xFFFF;

void printHelp() {
    std::cout <<
        "Usage:\n"
        "  killer.exe --id <PID>        Kill by PID\n"
        "  killer.exe --name <NAME>     Kill by image name\n"
        "  killer.exe                     Kill from PROC_TO_KILL\n";
}

void killByPid(DWORD pid) {
    if (pid < MIN_PID || pid > MAX_PID) return;
    HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (h) {
        TerminateProcess(h, 0);
        CloseHandle(h);
        std::cout << "[killer] Killed PID " << pid << '\n';
    }
    else {
        std::cout << "[killer] PID " << pid << " not found or access denied\n";
    }
}

void killByName(const char* target) {
    PROCESSENTRY32 pe{ sizeof(pe) };
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return;

    bool found = false;
    for (BOOL ok = Process32First(snap, &pe); ok; ok = Process32Next(snap, &pe)) {
#ifdef UNICODE
        char exeFile[MAX_PATH];
        WideCharToMultiByte(CP_ACP, 0, pe.szExeFile, -1, exeFile, MAX_PATH, nullptr, nullptr);
#else
        const char* exeFile = pe.szExeFile;
#endif
        if (_stricmp(exeFile, target) == 0) {
            found = true;
            HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
            if (h) {
                TerminateProcess(h, 0);
                CloseHandle(h);
                std::cout << "[killer] Killed " << exeFile << " (PID " << pe.th32ProcessID << ")\n";
            }
        }
    }
    if (!found) std::cout << "[killer] No process named \"" << target << "\" found\n";
    CloseHandle(snap);
}

void killFromEnv() {
    char buf[4096];
    DWORD len = GetEnvironmentVariableA("PROC_TO_KILL", buf, sizeof(buf));
    if (!len || len > sizeof(buf)) {
        std::cout << "[killer] PROC_TO_KILL empty or too large\n";
        return;
    }
    std::stringstream ss(buf);
    std::string name;
    while (std::getline(ss, name, ',')) {
        if (!name.empty()) killByName(name.c_str());
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) { killFromEnv(); return 0; }
    if (argc == 2 && strcmp(argv[1], "--help") == 0) { printHelp(); return 0; }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--id") == 0 && i + 1 < argc) {
            killByPid(static_cast<DWORD>(std::stoul(argv[++i])));
        }
        else if (strcmp(argv[i], "--name") == 0 && i + 1 < argc) {
            killByName(argv[++i]);
        }
    }
    killFromEnv();
    return 0;
}