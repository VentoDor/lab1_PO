#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#define SeedNum 7
#define ShouldCheckCorrectness 1

using namespace std;
using chrono::nanoseconds;
using chrono::duration_cast;
using chrono::high_resolution_clock;


void printSystemInfo(int &cpuNum) {
    cpuNum = thread::hardware_concurrency(); 
    cout << "System Information:" << endl;
    cout << "Number of logical processors: " << cpuNum << endl;
}


void processMatrixSection(int startRow, int endRow,
                          const vector<vector<int>> &A,
                          const vector<vector<int>> &B,
                          vector<vector<int>> &C) {
    for (int i = startRow; i < endRow; ++i) {
        for (int j = 0; j < A.size(); ++j) {
            C[i][j] = A[i][j] + B[i][j]; 
        }
    }
}


void linearProcessMatrix(const vector<vector<int>> &A,
                         const vector<vector<int>> &B,
                         vector<vector<int>> &C) {
    for (int i = 0; i < C.size(); ++i) {
        for (int j = 0; j < C.size(); ++j) {
            C[i][j] = A[i][j] + B[i][j]; 
        }
    }
}


bool checkMatrixCorrectness(const vector<vector<int>> &C,
                            const vector<vector<int>> &A,
                            const vector<vector<int>> &B,
                            int randomRowCount = 10) {
    vector<int> randomRows;
    for (int i = 0; i < randomRowCount; ++i) {
        randomRows.push_back(rand() % C.size());
    }

    bool isCorrect = true;
    for (auto row : randomRows) {
        if (row >= C.size()) continue;
        for (int j = 0; j < C.size(); ++j) {
            int expected = A[row][j] + B[row][j];
            if (C[row][j] != expected) {
                cout << "Error in row " << row << ", col " << j
                     << ": Expected " << expected
                     << ", but got " << C[row][j] << endl;
                isCorrect = false;
            }
        }
    }
    return isCorrect;
}

int main() {
    int cpuNum;
    printSystemInfo(cpuNum);

    vector<int> matrixSizes = {100, 1000, 5000, 10000};
    vector<int> numCPUArr = {
        cpuNum / 2,
        cpuNum,
        cpuNum * 2,
        cpuNum * 4,
        cpuNum * 8,
        cpuNum * 16,
    };

    cout << "\nTest Results:" << endl;
    cout << "Matrix Size\tThreads\tTime (seconds)\tCorrect?" << endl;

    for (int matrixSize : matrixSizes) {

        vector<vector<int>> A(matrixSize, vector<int>(matrixSize));
        vector<vector<int>> B(matrixSize, vector<int>(matrixSize));
        srand(SeedNum);

        for (int i = 0; i < matrixSize; ++i) {
            for (int j = 0; j < matrixSize; ++j) {
                A[i][j] = rand() % 10001;
                B[i][j] = rand() % 10001;
            }
        }

        {
            vector<vector<int>> C(matrixSize, vector<int>(matrixSize));
            auto start = high_resolution_clock::now();
            linearProcessMatrix(A, B, C);
            auto end = high_resolution_clock::now();

            string correctness = ShouldCheckCorrectness
                                     ? (checkMatrixCorrectness(C, A, B) ? "Yes" : "No")
                                     : "Unknown";
            auto elapsed = duration_cast<nanoseconds>(end - start).count() * 1e-9;

            cout << endl
                 << matrixSize << "\t\tLinear\t" << fixed << setprecision(6)
                 << elapsed << "\t" << correctness << endl;
        }

        for (int threadsCount : numCPUArr) {
            vector<vector<int>> C(matrixSize, vector<int>(matrixSize));
            vector<thread> threads;
            auto start = high_resolution_clock::now();

            int rowsPerThread = matrixSize / threadsCount;
            int extraRows = matrixSize % threadsCount;

            for (int t = 0; t < threadsCount; ++t) {
                int startRow = t * rowsPerThread + min(t, extraRows);
                int endRow = startRow + rowsPerThread + (t < extraRows ? 1 : 0);

                threads.emplace_back(processMatrixSection, startRow, endRow,
                                     cref(A), cref(B), ref(C));
            }

            for (auto &th : threads) {
                if (th.joinable()) {
                    th.join();
                }
            }

            auto end = high_resolution_clock::now();
            string correctness = ShouldCheckCorrectness
                                     ? (checkMatrixCorrectness(C, A, B) ? "Yes" : "No")
                                     : "Unknown";
            auto elapsed = duration_cast<nanoseconds>(end - start).count() * 1e-9;

            cout << matrixSize << "\t\t" << threadsCount << "\t" << fixed
                 << setprecision(6) << elapsed << "\t" << correctness << endl;
        }
    }

    return 0;
}