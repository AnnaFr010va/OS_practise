#include "Matrix.h"
#include <random>
#include <stdexcept>
#include <vector>
#include <atomic>
#include <thread>
#include <pthread.h>

Matrix::Matrix(int rows_, int cols_) : rows(rows_), cols(cols_) {
    if (rows <= 0 || cols <= 0) throw std::invalid_argument("Matrix dimensions must be positive");
    data.resize(rows, std::vector<int>(cols, 0));
}

int& Matrix::operator()(int i, int j) {
    if (i < 0 || i >= rows || j < 0 || j >= cols) throw std::out_of_range("Matrix indices out of range");
    return data[i][j];
}
const int& Matrix::operator()(int i, int j) const {
    if (i < 0 || i >= rows || j < 0 || j >= cols) throw std::out_of_range("Matrix indices out of range");
    return data[i][j];
}

void Matrix::fillRandom() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 10);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            data[i][j] = dist(gen);
}
void Matrix::print() const {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) std::cout << data[i][j] << '\t';
        std::cout << '\n';
    }
}
bool Matrix::isEqual(const Matrix& other) const {
    if (rows != other.rows || cols != other.cols) return false;
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            if (data[i][j] != other(i, j)) return false;
    return true;
}

Matrix Matrix::simpleMultiply(const Matrix& a, const Matrix& b) {
    if (a.getCols() != b.getRows()) throw std::invalid_argument("Matrix dimensions don't match");
    int n = a.getRows(), m = b.getCols(), p = a.getCols();
    Matrix result(n, m);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j) {
            int sum = 0;
            for (int k = 0; k < p; ++k) sum += a(i, k) * b(k, j);
            result(i, j) = sum;
        }
    return result;
}

Matrix Matrix::blockMultiply(const Matrix& a, const Matrix& b, int blockSize) {
    if (a.getCols() != b.getRows()) throw std::invalid_argument("Matrix dimensions don't match");
    int n = a.getRows(), m = b.getCols(), p = a.getCols();
    Matrix result(n, m);
    for (int ii = 0; ii < n; ii += blockSize)
        for (int jj = 0; jj < m; jj += blockSize)
            for (int kk = 0; kk < p; kk += blockSize) {
                int i_end = std::min(ii + blockSize, n);
                int j_end = std::min(jj + blockSize, m);
                int k_end = std::min(kk + blockSize, p);
                for (int i = ii; i < i_end; ++i)
                    for (int j = jj; j < j_end; ++j) {
                        int sum = 0;
                        for (int k = kk; k < k_end; ++k) sum += a(i, k) * b(k, j);
                        result(i, j) += sum;
                    }
            }
    return result;
}

struct PThreadData {
    const Matrix* a;
    const Matrix* b;
    Matrix* result;
    int blockSize, blocksCol, n, m, p;
    std::atomic<int>* nextBlock;
};

static void* pthreadWorker(void* arg) {
    PThreadData* td = static_cast<PThreadData*>(arg);
    const Matrix& a = *td->a;
    const Matrix& b = *td->b;
    Matrix& result = *td->result;

    int k;
    while ((k = td->nextBlock->fetch_add(1, std::memory_order_relaxed)) <
           ((td->n + td->blockSize - 1) / td->blockSize) *
           ((td->m + td->blockSize - 1) / td->blockSize)) {

        int blockI = k / td->blocksCol;
        int blockJ = k % td->blocksCol;

        int iStart = blockI * td->blockSize;
        int iEnd   = std::min(iStart + td->blockSize, td->n);
        int jStart = blockJ * td->blockSize;
        int jEnd   = std::min(jStart + td->blockSize, td->m);

        for (int i = iStart; i < iEnd; ++i)
            for (int j = jStart; j < jEnd; ++j) {
                int sum = 0;
                for (int kk = 0; kk < td->p; ++kk) sum += a(i, kk) * b(kk, j);
                result(i, j) = sum;
            }
    }
    return nullptr;
}

Matrix Matrix::parallelMultiplyPthread(const Matrix& a, const Matrix& b, int blockSize) {
    if (a.getCols() != b.getRows()) throw std::invalid_argument("Matrix dimensions don't match");
    int n = a.getRows(), m = b.getCols(), p = a.getCols();
    Matrix result(n, m);

    int blocksRow = (n + blockSize - 1) / blockSize;
    int blocksCol = (m + blockSize - 1) / blockSize;
    int totalBlocks = blocksRow * blocksCol;

    unsigned int maxThreads = std::thread::hardware_concurrency();
    if (maxThreads == 0) maxThreads = 4;
    if (maxThreads > 16) maxThreads = 16;
    if (totalBlocks < maxThreads) maxThreads = totalBlocks;

    std::atomic<int> nextBlock(0);
    PThreadData common{ &a, &b, &result, blockSize, blocksCol, n, m, p, &nextBlock };

    std::vector<pthread_t> threads;
    threads.reserve(maxThreads);
    for (unsigned int t = 0; t < maxThreads; ++t) {
        pthread_t th;
        pthread_create(&th, nullptr, pthreadWorker, &common);
        threads.push_back(th);
    }
    for (pthread_t th : threads) pthread_join(th, nullptr);
    return result;
}

