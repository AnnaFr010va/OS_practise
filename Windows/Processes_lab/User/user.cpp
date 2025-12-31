#include <windows.h>
#include <iostream>
#include <tlhelp32.h>
#include <cstring>
#include <vector>
#include <sstream>
#include <string>
#include <cstdlib>

DWORD getProcessIdByName(const char* processName) {
    PROCESSENTRY32 entry{};
    entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;

    DWORD pid = 0;
    if (Process32First(snapshot, &entry)) {
        do {
#ifdef UNICODE
            char exeFile[MAX_PATH];
            WideCharToMultiByte(CP_ACP, 0, entry.szExeFile, -1, exeFile, MAX_PATH, nullptr, nullptr);
#else
            const char* exeFile = entry.szExeFile;
#endif
            if (_stricmp(exeFile, processName) == 0) {
                pid = entry.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return pid;
}

bool isProcessRunning(DWORD pid) {
    if (!pid) return false;
    HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!h) return false;
    DWORD code = 0;
    GetExitCodeProcess(h, &code);
    CloseHandle(h);
    return code == STILL_ACTIVE;
}

bool isProcessRunning(const char* name) {
    return isProcessRunning(getProcessIdByName(name));
}

bool runKiller(const std::string& args) {
    std::string cmd = "killer.exe " + args;
    STARTUPINFOA si{ sizeof(si) };
    PROCESS_INFORMATION pi{};
    std::cout << "Running: " << cmd << std::endl;
    if (!CreateProcessA(nullptr, const_cast<char*>(cmd.c_str()), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
        return false;
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return exitCode == 0;
}

bool waitUntilProcessDead(DWORD pid, const char* name, int timeoutMs = 5000) {
    int checks = timeoutMs / 100;
    while (checks-- > 0) {
        bool dead = true;
        if (pid && isProcessRunning(pid)) dead = false;
        if (name && isProcessRunning(name)) dead = false;
        if (dead) return true;
        Sleep(100);
    }
    return false;
}

void checkProcesses(DWORD pid, const char* name) {
    char buffer[4096];
    if (GetEnvironmentVariableA("PROC_TO_KILL", buffer, sizeof(buffer))) {
        std::stringstream ss(buffer);
        std::string item;
        while (std::getline(ss, item, ',')) {
            if (!item.empty())
                std::cout << "from PROC_TO_KILL: " << item
                << (isProcessRunning(item.c_str()) ? " RUNNING" : " NOT RUNNING") << '\n';
        }
    }
    else {
        std::cout << "PROC_TO_KILL not found\n";
    }
    if (name) std::cout << name << (isProcessRunning(name) ? " RUNNING" : " NOT RUNNING") << '\n';
    if (pid) std::cout << "process with PID " << pid << (isProcessRunning(pid) ? " RUNNING" : " NOT RUNNING") << '\n';
}

void testScenario(const std::string& desc, const std::string& killerArgs, DWORD pid = 0, const char* name = nullptr) {
    std::cout << "\n=== " << desc << " ===\nBEFORE:\n";
    checkProcesses(pid, name);
    if (runKiller(killerArgs)) {
        std::cout << "Waiting for process to die...\n";
        if (waitUntilProcessDead(pid, name))
            std::cout << "AFTER (confirmed dead):\n";
        else
            std::cout << "AFTER (still running or timeout):\n";
        checkProcesses(pid, name);
    }
    else {
        std::cout << "Killer failed!\n";
    }
}

void launchProcess(const char* proc) {
    std::string cmd = "start " + std::string(proc);
    system(cmd.c_str());
    std::cout << "Launched: " << proc << '\n';
    Sleep(1000);
}

int main() {

    const char* list = "notepad.exe,mspaint.exe,Calculator.exe,CalculatorApp.exe";
    SetEnvironmentVariableA("PROC_TO_KILL", list);
    std::cout << "Set PROC_TO_KILL = " << list << '\n';

    std::cout << "\n--- Launching test processes ---\n";
    launchProcess("notepad.exe");
    launchProcess("mspaint.exe");
    launchProcess("calc.exe");
    Sleep(2000);

    DWORD np = getProcessIdByName("notepad.exe");
    DWORD mp = getProcessIdByName("mspaint.exe");

    testScenario("Test 1: Killing by ID", "--id " + std::to_string(np), np, "notepad.exe");
    testScenario("Test 2: Killing by name", "--name calc.exe", 0, "calc.exe");
    testScenario("Test 3: Killing from PROC_TO_KILL", "", mp, "mspaint.exe");

    std::cout << "\n=== All tests completed ===\n";
    return 0;
}