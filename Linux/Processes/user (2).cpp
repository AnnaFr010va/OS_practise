#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <dirent.h>
#include <sys/types.h>

bool isProcRunning(pid_t pid) {
    return kill(pid, 0) == 0;
}

bool isProcRunning(const char* name) {
    DIR* dir = opendir("/proc");
    if (!dir) return false;
    dirent* entry;
    while ((entry = readdir(dir))) {
        if (!isdigit(entry->d_name[0])) continue;
        std::string path = "/proc/" + std::string(entry->d_name) + "/comm";
        std::ifstream comm(path);
        std::string procName;
        std::getline(comm, procName);
        if (procName == name) { closedir(dir); return true; }
    }
    closedir(dir);
    return false;
}

pid_t getPidByName(const char* name) {
    DIR* dir = opendir("/proc");
    if (!dir) return 0;
    dirent* entry;
    while ((entry = readdir(dir))) {
        if (!isdigit(entry->d_name[0])) continue;
        std::string path = "/proc/" + std::string(entry->d_name) + "/comm";
        std::ifstream comm(path);
        std::string procName;
        std::getline(comm, procName);
        if (procName == name) {
            closedir(dir);
            return std::stoi(entry->d_name);
        }
    }
    closedir(dir);
    return 0;
}

bool runKiller(const std::string& args) {
    std::string cmd = "./killer " + args;
    std::cout << "Running: " << cmd << std::endl;
    return system(cmd.c_str()) == 0;
}

bool waitUntilProcDead(pid_t pid, const char* name, int timeoutMs = 5000) {
    int checks = timeoutMs / 100;
    while (checks-- > 0) {
        bool dead = true;
        if (pid && isProcRunning(pid)) dead = false;
        if (name && isProcRunning(name)) dead = false;
        if (dead) return true;
        usleep(100000);
    }
    return false;
}

void checkProcs(pid_t pid, const char* name) {
    const char* env = std::getenv("PROC_TO_KILL");
    if (env) {
        std::stringstream ss(env);
        std::string item;
        while (std::getline(ss, item, ',')) {
            if (!item.empty())
                std::cout << "from PROC_TO_KILL: " << item
                << (isProcRunning(item.c_str()) ? " RUNNING" : " NOT RUNNING") << '\n';
        }
    } else {
        std::cout << "PROC_TO_KILL not found\n";
    }
    if (name) std::cout << name << (isProcRunning(name) ? " RUNNING" : " NOT RUNNING") << '\n';
    if (pid) std::cout << "process with PID " << pid << (isProcRunning(pid) ? " RUNNING" : " NOT RUNNING") << '\n';
}

void testScenario(const std::string& desc, const std::string& killerArgs, pid_t pid = 0, const char* name = nullptr) {
    std::cout << "\n=== " << desc << " ===\nBEFORE:\n";
    checkProcs(pid, name);
    if (runKiller(killerArgs)) {
        std::cout << "Waiting for process to die...\n";
        if (waitUntilProcDead(pid, name))
            std::cout << "AFTER (confirmed dead):\n";
        else
            std::cout << "AFTER (still running or timeout):\n";
        checkProcs(pid, name);
    } else {
        std::cout << "Killer failed!\n";
    }
}

void launchProc(const char* name) {
    std::string cmd = name;
    if (std::strstr(name, "gedit") || std::strstr(name, "xterm") || std::strstr(name, "gnome-calculator"))
        cmd += " &";
    std::system(cmd.c_str());
    std::cout << "Launched: " << name << '\n';
    sleep(2);
}

int main() {
    const char* list = "xterm,gedit,gnome-calculator";
    setenv("PROC_TO_KILL", list, 1);
    std::cout << "Set PROC_TO_KILL = " << list << '\n';

    std::cout << "\n--- Launching test processes ---\n";
    launchProc("xterm");
    launchProc("gedit");
    launchProc("gnome-calculator");
    sleep(3);

    pid_t xterm  = getPidByName("xterm");
    pid_t gedit  = getPidByName("gedit");

    testScenario("Test 1: Killing by PID", "--id " + std::to_string(xterm), xterm, "xterm");
    testScenario("Test 2: Killing by name", "--name gnome-calculator", 0, "gnome-calculator");
    testScenario("Test 3: Killing from PROC_TO_KILL", "", gedit, "gedit");

    std::cout << "\n=== All tests completed ===\n";
    return 0;
}

