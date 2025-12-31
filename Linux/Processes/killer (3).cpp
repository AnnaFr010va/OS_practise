#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <csignal>
#include <dirent.h>
#include <sys/types.h>

void printHelp() {
    std::cout <<
        "Usage:\n"
        "  ./killer --id PID        Kill by PID\n"
        "  ./killer --name NAME     Kill by /proc/PID/comm name\n"
        "  ./killer                   Kill from PROC_TO_KILL\n";
}

bool killPid(pid_t pid) {
    if (kill(pid, SIGTERM) == 0) {
        std::cout << "[killer] Killed PID " << pid << '\n';
        return true;
    } else {
        std::cout << "[killer] PID " << pid << " not found or permission denied\n";
        return false;
    }
}

void killByName(const char* target) {
    DIR* dir = opendir("/proc");
    if (!dir) return;
    bool found = false;
    dirent* entry;
    while ((entry = readdir(dir))) {
        if (!isdigit(entry->d_name[0])) continue;
        std::string path = "/proc/" + std::string(entry->d_name) + "/comm";
        std::ifstream comm(path);
        std::string name;
        std::getline(comm, name);
        if (name == target) {
            found = true;
            pid_t pid = std::stoi(entry->d_name);
            killPid(pid);
        }
    }
    if (!found) std::cout << "[killer] No process named \"" << target << "\" found\n";
    closedir(dir);
}

void killFromEnv() {
    const char* env = std::getenv("PROC_TO_KILL");
    if (!env || !*env) {
        std::cout << "[killer] PROC_TO_KILL empty or not set\n";
        return;
    }
    std::stringstream ss(env);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (!item.empty()) killByName(item.c_str());
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) { killFromEnv(); return 0; }
    if (argc == 2 && std::strcmp(argv[1], "--help") == 0) { printHelp(); return 0; }

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--id") == 0 && i + 1 < argc) {
            pid_t pid = std::stoi(argv[++i]);
            killPid(pid);
        } else if (std::strcmp(argv[i], "--name") == 0 && i + 1 < argc) {
            killByName(argv[++i]);
        }
    }
    killFromEnv();
    return 0;
}

