#include "Matrix.hpp"
#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
using namespace std;
using namespace Kelly;

void TestBasicMultiplication()
{
    auto a = Matrix<int>::RowMajor(2, 3);
    auto b = Matrix<int>::RowMajor(3, 2);

    for (int i = 0; i < 6; ++i) a[i] = i + 1;
    for (int i = 0; i < 6; ++i) b[i] = i + 7;

    cout << (a * b);
}

void MultiplyHugeMatrices()
{
    ofstream fout("matrix_result.txt", ofstream::binary);

    if (!fout)
    {
        cout << "Failed to open log file!" << endl;
        return;
    }

    mt19937_64 generator;
    uniform_int_distribution<int> distribution(-255, 255);

    const size_t N = 3000;
    auto a = Matrix<int>::RowMajor(N, N);
    auto b = Matrix<int>::ColumnMajor(N, N);

    cout << "Generating matrices..." << endl;

    for (size_t i = 0; i < a.CellCount(); ++i)
    {
        a[i] = distribution(generator);
        b[i] = distribution(generator);
    }

    fout << "[MATRIX A]\n" << a << "\n[MATRIX B]\n" << b;

    cout << "Multiplying matrices..." << endl;

    auto start = chrono::steady_clock::now();
    auto c = a * b; // This is where the magic happens.
    auto finish = chrono::steady_clock::now();
    auto span = chrono::duration_cast<chrono::duration<double>>(finish - start);

    cout << "Finished in " << span.count() << " seconds." << endl;

    fout << "\n[MATRIX C]\n" << c << "\nFinished in " << span.count()
        << " seconds." << endl;

    fout.close();
}

int main(int argc, char** argv)
{
    //TestBasicMultiplication();
    MultiplyHugeMatrices();
    return 0;
}
