#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <iostream>

class Matrix {
private:
    std::vector<std::vector<int>> data;
    int rows;
    int cols;

public:
    Matrix(int rows, int cols);

    int getRows() const { return rows; }
    int getCols() const { return cols; }
    int& operator()(int i, int j);
    const int& operator()(int i, int j) const;

    void fillRandom();
    void print() const;
    bool isEqual(const Matrix& other) const;

    static Matrix simpleMultiply(const Matrix& a, const Matrix& b);
    static Matrix blockMultiply(const Matrix& a, const Matrix& b, int blockSize);
    static Matrix parallelMultiply(const Matrix& a, const Matrix& b, int blockSize);
};

#endif