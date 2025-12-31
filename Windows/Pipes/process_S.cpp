#include <iostream>

int main() {
    int x;
    long long sum = 0;
    while (std::cin >> x) {
        sum += x;
    }
    std::cout << sum << std::endl;
    return 0;
}