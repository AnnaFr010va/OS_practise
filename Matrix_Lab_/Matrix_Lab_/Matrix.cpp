#include "Matrix.h"
#include <random>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <iostream>

Matrix::Matrix(int rows_, int cols_) : rows(rows_), cols(cols_) {
    if (rows <= 0 || cols <= 0) {
        throw std::invalid_argument("Matrix dimensions must be positive");
    }
    data.resize(rows, std::vector<int>(cols, 0));
}

int& Matrix::operator()(int i, int j) {
    if (i < 0 || i >= rows || j < 0 || j >= cols) {
        throw std::out_of_range("Matrix indices out of range");
    }
    return data[i][j];
}

const int& Matrix::operator()(int i, int j) const {
    if (i < 0 || i >= rows || j < 0 || j >= cols) {
        throw std::out_of_range("Matrix indices out of range");
    }
    return data[i][j];
}

void Matrix::fillRandom() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 10);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            data[i][j] = dist(gen);
        }
    }
}

void Matrix::print() const {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << data[i][j] << "\t";
        }
        std::cout << std::endl;
    }
}

bool Matrix::isEqual(const Matrix& other) const {
    if (rows != other.rows || cols != other.cols) return false;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (data[i][j] != other(i, j)) {
                return false;
            }
        }
    }
    return true;
}

Matrix Matrix::simpleMultiply(const Matrix& a, const Matrix& b) {
    if (a.getCols() != b.getRows()) {
        throw std::invalid_argument("Matrix dimensions don't match");
    }

    int n = a.getRows();
    int m = b.getCols();
    int p = a.getCols();

    Matrix result(n, m);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            int sum = 0;
            for (int k = 0; k < p; ++k) {
                sum += a(i, k) * b(k, j);
            }
            result(i, j) = sum;
        }
    }

    return result;
}

Matrix Matrix::blockMultiply(const Matrix& a, const Matrix& b, int blockSize) {
    if (a.getCols() != b.getRows()) {
        throw std::invalid_argument("Matrix dimensions don't match");
    }

    int n = a.getRows();
    int m = b.getCols();
    int p = a.getCols();

    Matrix result(n, m);

    for (int ii = 0; ii < n; ii += blockSize) {
        for (int jj = 0; jj < m; jj += blockSize) {
            for (int kk = 0; kk < p; kk += blockSize) {
                int i_end = std::min(ii + blockSize, n);
                int j_end = std::min(jj + blockSize, m);
                int k_end = std::min(kk + blockSize, p);

                for (int i = ii; i < i_end; ++i) {
                    for (int j = jj; j < j_end; ++j) {
                        int sum = 0;
                        for (int k = kk; k < k_end; ++k) {
                            sum += a(i, k) * b(k, j);
                        }
                        result(i, j) += sum;
                    }
                }
            }
        }
    }

    return result;
}

Matrix Matrix::parallelMultiply(const Matrix& a, const Matrix& b, int blockSize) {
    if (a.getCols() != b.getRows()) {
        throw std::invalid_argument("Matrix dimensions don't match");
    }

    int n = a.getRows();
    int m = b.getCols();
    int p = a.getCols();

    Matrix result(n, m);

    int blocksRow = (n + blockSize - 1) / blockSize;
    int blocksCol = (m + blockSize - 1) / blockSize;
    int totalBlocks = blocksRow * blocksCol;

    unsigned int maxThreads = std::thread::hardware_concurrency();
    if (maxThreads == 0) maxThreads = 4;
    if (maxThreads > 16) maxThreads = 16;
    if (totalBlocks < maxThreads) maxThreads = totalBlocks;

    std::vector<std::thread> threads;
    threads.reserve(maxThreads);

    for (unsigned int threadId = 0; threadId < maxThreads; ++threadId) {
        threads.emplace_back([&, threadId]() {
            for (int blockIdx = threadId; blockIdx < totalBlocks; blockIdx += maxThreads) {
                int blockI = blockIdx / blocksCol;
                int blockJ = blockIdx % blocksCol;

                int iStart = blockI * blockSize;
                int iEnd = std::min(iStart + blockSize, n);
                int jStart = blockJ * blockSize;
                int jEnd = std::min(jStart + blockSize, m);

                for (int i = iStart; i < iEnd; ++i) {
                    for (int j = jStart; j < jEnd; ++j) {
                        int sum = 0;
                        for (int k = 0; k < p; ++k) {
                            sum += a(i, k) * b(k, j);
                        }
                        result(i, j) = sum;
                    }
                }
            }
            });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return result;
}