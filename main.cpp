#include "Matrix.hpp"
#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include <thread>
#include <sstream>
using namespace std;
using namespace Kelly;

#define LDB cerr << "yarr -- " << __LINE__ << '\n'

void TestBasicMultiplication()
{
    auto a = Matrix<int>::RowMajor(2, 3);
    auto b = Matrix<int>::RowMajor(3, 2);

    for (int i = 0; i < 6; ++i) a[i] = i + 1;
    for (int i = 0; i < 6; ++i) b[i] = i + 7;

    cout << (a * b);
}

struct MathJob
{
    size_t startIndex;
    size_t cellCount;
    const Matrix<int>* a;
    const Matrix<int>* b;
    Matrix<int>* c;
};

void JobThread(MathJob job)
{
    const auto& a = *job.a;
    const auto& b = *job.b;
    auto& c = *job.c;
    size_t row = job.startIndex / c.ColumnCount();
    size_t column = job.startIndex % c.ColumnCount();

    for (size_t i = row; i < c.RowCount(); ++i)
    {
        for (size_t j = column; j < c.ColumnCount(); ++j)
        {
            if (job.cellCount < 1) return;
            --job.cellCount;

            int total = 0;

            for (size_t k = 0; k < a.ColumnCount(); ++k)
            {
                total += a(i, k) * b(k, j);
            }

            c(i, j) = total;
        }

        column = 0;
    }
}

void DoThreadedMultiply(size_t matrixEdge, size_t threadCount, bool checkAnswer)
{
    if (matrixEdge < 1) matrixEdge = 1;
    if (threadCount < 1) threadCount = 1;

    mt19937_64 generator;
    uniform_int_distribution<int> distribution(-255, 255);

    auto a = Matrix<int>::RowMajor(matrixEdge, matrixEdge);
    auto b = Matrix<int>::ColumnMajor(matrixEdge, matrixEdge);
    auto c = Matrix<int>::RowMajor(matrixEdge, matrixEdge);

    for (size_t i = 0; i < a.CellCount(); ++i)
    {
        a[i] = distribution(generator);
        b[i] = distribution(generator);
    }

    cout << "Multiplying matrices..." << endl;

    auto start = chrono::steady_clock::now();

    vector<MathJob> jobs(threadCount);
    size_t minCellsPerThread = c.CellCount() / threadCount;
    size_t excessCellCount = c.CellCount() % threadCount;

    size_t startIndex = 0;
    for (size_t i = 0; i < jobs.size(); ++i)
    {
        MathJob job = {
            startIndex,
            minCellsPerThread + (i < excessCellCount),
            &a, &b, &c };

        jobs[i] = job;

        startIndex += job.cellCount;
    }

    vector<thread> threads;

    for (auto job : jobs)
    {
        if (job.cellCount > 0)
            threads.push_back(thread(JobThread, job));
    }

    for (auto& t : threads) t.join();

    auto finish = chrono::steady_clock::now();
    auto span = chrono::duration_cast<chrono::duration<double>>(finish - start);

    cout << "Finished in " << span.count() << " seconds using " << threads.size() << " threads." << endl;

    if (checkAnswer)
    {
        cout << "Checking answer..." << endl;

        auto d = a * b;

        if (c == d)
        {
            cout << "Answer is correct." << endl;
        }
        else
        {
            cout
                << "[A]\n"
                << a
                << "[B]\n"
                << b
                << "[EXPECTED]\n"
                << d
                << "[ACTUAL]\n"
                << c;
        }
    }
}

void MultiplyHugeMatrices(bool cacheFriendly)
{
    ofstream fout("matrix_result.txt", ofstream::binary);

    if (!fout)
    {
        cout << "Failed to open log file!" << endl;
        return;
    }

    auto seed = chrono::system_clock::now().time_since_epoch().count();
    mt19937_64 generator(seed);
    uniform_int_distribution<int> distribution(-255, 255);

    const size_t N = 1000;
    auto a = Matrix<int>::RowMajor(N, N);
    auto b = Matrix<int>::ColumnMajor(N, N);

    if (!cacheFriendly)
    {
        a.ReinterpretAsColumnMajor();
        b.ReinterpretAsRowMajor();
    }

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
    //MultiplyHugeMatrices(true);
    //MultiplyHugeMatrices(false);
    //return 0;

    if (argc > 2)
    {
        stringstream ss;
        ss << argv[1] << ' ' << argv[2];

        size_t matrixEdge = 0;
        size_t threadCount = 0;

        if (ss >> matrixEdge >> threadCount)
            DoThreadedMultiply(matrixEdge, threadCount, false);
        else
            cerr << "invalid arguments\n";
    }
    else
    {
        cerr << "usage: " << argv[0] << " <matrix-size> <thread-count>";
    }

    return 0;
}
