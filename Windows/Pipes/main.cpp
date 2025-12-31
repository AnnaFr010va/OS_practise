#include <windows.h>
#include <iostream>
#include <string>
#include <cstring>

void build_children()
{
    system("g++ -o process_M.exe process_M.cpp");
    system("g++ -o process_A.exe process_A.cpp");
    system("g++ -o process_P.exe process_P.cpp");
    system("g++ -o process_S.exe process_S.cpp");
}

HANDLE run(const char* exe, HANDLE hStdIn, HANDLE hStdOut, HANDLE hStdErr = nullptr)
{
    STARTUPINFOA si{ sizeof(si) };
    PROCESS_INFORMATION pi{ 0 };
    si.dwFlags     = STARTF_USESTDHANDLES;
    si.hStdInput   = hStdIn;
    si.hStdOutput  = hStdOut;
    si.hStdError   = hStdErr ? hStdErr : GetStdHandle(STD_ERROR_HANDLE);

    char cmd[256];
    std::snprintf(cmd, sizeof(cmd), "%s.exe", exe);

    if (!CreateProcessA(nullptr, cmd, nullptr, nullptr, TRUE,
                        0, nullptr, nullptr, &si, &pi))
    {
        std::cerr << "CreateProcess " << cmd << " failed: " << GetLastError() << '\n';
        return nullptr;
    }
    CloseHandle(pi.hThread);
    return pi.hProcess;
}

int main()
{
    build_children();

    HANDLE hRead0,  hWrite0;
    HANDLE hRead1,  hWrite1;
    HANDLE hRead2,  hWrite2;
    HANDLE hRead3,  hWrite3;
    HANDLE hReadFinal, hWriteFinal;

    SECURITY_ATTRIBUTES sa{ sizeof(sa), nullptr, TRUE };

    if (!CreatePipe(&hRead0,  &hWrite0,  &sa, 0) ||
        !CreatePipe(&hRead1,  &hWrite1,  &sa, 0) ||
        !CreatePipe(&hRead2,  &hWrite2,  &sa, 0) ||
        !CreatePipe(&hRead3,  &hWrite3,  &sa, 0) ||
        !CreatePipe(&hReadFinal, &hWriteFinal, &sa, 0))
    {
        std::cerr << "CreatePipe failed\n";
        return 1;
    }

    SetHandleInformation(hRead0,  HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hWrite0, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hRead1,  HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hWrite1, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hRead2,  HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hWrite2, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hRead3,  HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hWrite3, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hReadFinal,  HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hWriteFinal, HANDLE_FLAG_INHERIT, 0);

    SetHandleInformation(hRead0,  HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    SetHandleInformation(hWrite1, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    HANDLE hProcM = run("process_M", hRead0, hWrite1);
    SetHandleInformation(hRead0,  HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hWrite1, HANDLE_FLAG_INHERIT, 0);

    SetHandleInformation(hRead1,  HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    SetHandleInformation(hWrite2, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    HANDLE hProcA = run("process_A", hRead1, hWrite2);
    SetHandleInformation(hRead1,  HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hWrite2, HANDLE_FLAG_INHERIT, 0);

    SetHandleInformation(hRead2,  HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    SetHandleInformation(hWrite3, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    HANDLE hProcP = run("process_P", hRead2, hWrite3);
    SetHandleInformation(hRead2,  HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hWrite3, HANDLE_FLAG_INHERIT, 0);

    SetHandleInformation(hRead3,  HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    SetHandleInformation(hWriteFinal, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    HANDLE hProcS = run("process_S", hRead3, hWriteFinal);
    SetHandleInformation(hRead3,  HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hWriteFinal, HANDLE_FLAG_INHERIT, 0);

    if (!hProcM || !hProcA || !hProcP || !hProcS)
        return 1;

    std::string line;
    std::getline(std::cin, line);
    if (line.empty() || line.back() != '\n') line += '\n';

    DWORD dw;
    WriteFile(hWrite0, line.c_str(), line.size(), &dw, nullptr);
    CloseHandle(hWrite0);

    CloseHandle(hRead0);
    CloseHandle(hRead1);  CloseHandle(hWrite1);
    CloseHandle(hRead2);  CloseHandle(hWrite2);
    CloseHandle(hRead3);  CloseHandle(hWrite3);
    CloseHandle(hWriteFinal);

    HANDLE procs[4] = { hProcM, hProcA, hProcP, hProcS };
    WaitForMultipleObjects(4, procs, TRUE, INFINITE);
    for (auto h : procs) CloseHandle(h);

    std::string answer;
    char buf[512];
    DWORD bytesRead;
    while (ReadFile(hReadFinal, buf, sizeof(buf)-1, &bytesRead, nullptr) && bytesRead)
    {
        buf[bytesRead] = 0;
        answer += buf;
    }
    CloseHandle(hReadFinal);

    std::cout << answer;
    return 0;
}