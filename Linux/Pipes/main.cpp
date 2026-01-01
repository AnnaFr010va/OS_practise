#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

void build_children()
{
    system("g++ -std=c++17 -o process_M process_M.cpp");
    system("g++ -std=c++17 -o process_A process_A.cpp");
    system("g++ -std=c++17 -o process_P process_P.cpp");
    system("g++ -std=c++17 -o process_S process_S.cpp");
}

pid_t run(const char* exe, int fd_in, int fd_out)
{
    pid_t pid = fork();
    if (pid == 0) {
        if (fd_in  != STDIN_FILENO)  { dup2(fd_in,  STDIN_FILENO);  close(fd_in);  }
        if (fd_out != STDOUT_FILENO) { dup2(fd_out, STDOUT_FILENO); close(fd_out); }
        execlp(exe, exe, nullptr);
        std::cerr << "execlp " << exe << " failed\n";
        _exit(1);
    }
    if (pid < 0) std::cerr << "fork failed\n";
    return pid;
}

int main()
{
    build_children();

    int p1[2], p2[2], p3[2], p4[2], p5[2];
    if (pipe(p1) || pipe(p2) || pipe(p3) || pipe(p4) || pipe(p5)) {
        std::cerr << "pipe failed\n";
        return 1;
    }

    for (int* pp : {p1, p2, p3, p4, p5})
        for (int i = 0; i < 2; ++i)
            fcntl(pp[i], F_SETFD, fcntl(pp[i], F_GETFD) | FD_CLOEXEC);

    pid_t pidM = run("./process_M", p1[0], p2[1]);
    close(p1[0]); close(p2[1]);
    if (pidM < 0) return 1;

    pid_t pidA = run("./process_A", p2[0], p3[1]);
    close(p2[0]); close(p3[1]);
    if (pidA < 0) return 1;

    pid_t pidP = run("./process_P", p3[0], p4[1]);
    close(p3[0]); close(p4[1]);
    if (pidP < 0) return 1;

    pid_t pidS = run("./process_S", p4[0], p5[1]);
    close(p4[0]); close(p5[1]);
    if (pidS < 0) return 1;

    std::string line;
    std::getline(std::cin, line);
    line += '\n';

    ssize_t total = 0;
    while (total < static_cast<ssize_t>(line.size())) {
        ssize_t w = write(p1[1], line.data() + total, line.size() - total);
        if (w <= 0) break;
        total += w;
    }
    close(p1[1]);

    std::vector<pid_t> kids = { pidM, pidA, pidP, pidS };
    for (pid_t pid : kids) waitpid(pid, nullptr, 0);

    std::string ans;
    char buf[512];
    ssize_t n;
    while ((n = read(p5[0], buf, sizeof(buf))) > 0)
        ans.append(buf, n);
    close(p5[0]);

    std::cout << ans;
    return 0;
}
