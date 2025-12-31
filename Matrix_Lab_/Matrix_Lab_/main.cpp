#include "Matrix.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <thread>

int main() {
    int n;
    std::cout << "Enter matrix size N (>=5): ";
    if (!(std::cin >> n) || n < 5) return 1;

    Matrix A(n, n), B(n, n);
    A.fillRandom();
    B.fillRandom();

    auto start = std::chrono::high_resolution_clock::now();
    Matrix ref = Matrix::simpleMultiply(A, B);
    auto end = std::chrono::high_resolution_clock::now();
    double refTime = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "\nMatrix " << n << "x" << n << " multiplication test\n";
    std::cout << "Simple multiplication time: " << std::fixed << std::setprecision(2) << refTime << " ms\n";
    std::cout << std::string(60, '-') << "\n";

    std::cout << std::left
        << std::setw(8) << "k"
        << std::setw(12) << "Blocks"
        << std::setw(12) << "Threads"
        << std::setw(12) << "SeqTime(ms)"
        << std::setw(12) << "ParTime(ms)"
        << std::setw(8) << "Check"
        << "\n";
    std::cout << std::string(60, '-') << "\n";

    for (int k = 1; k <= n; ++k) {
        int blocksRow = (n + k - 1) / k;
        int blocksCol = (n + k - 1) / k;
        int totalBlocks = blocksRow * blocksCol;
        unsigned int threads = totalBlocks;

        start = std::chrono::high_resolution_clock::now();
        Matrix seqResult = Matrix::blockMultiply(A, B, k);
        end = std::chrono::high_resolution_clock::now();
        double seqTime = std::chrono::duration<double, std::milli>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        Matrix parResult = Matrix::parallelMultiply(A, B, k);
        end = std::chrono::high_resolution_clock::now();
        double parTime = std::chrono::duration<double, std::milli>(end - start).count();

        bool correct = seqResult.isEqual(ref) && parResult.isEqual(ref);

        std::cout << std::left
            << std::setw(8) << k
            << std::setw(12) << totalBlocks
            << std::setw(12) << threads
            << std::fixed << std::setprecision(2)
            << std::setw(12) << seqTime
            << std::setw(12) << parTime
            << (correct ? "OK" : "FAIL")
            << "\n";
    }

    std::cout << std::string(60, '-') << "\n";
    std::cout << "Done!\n";

    return 0;
}