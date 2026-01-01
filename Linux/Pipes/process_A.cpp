#include <iostream>

const int N = 22;
int main() {
    int x;
    while (std::cin >> x) {
        std::cout << x + N << '\n';
    }
    return 0;
}
