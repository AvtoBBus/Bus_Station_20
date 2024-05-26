#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cmath>
#include <chrono>
#include <vector>

using namespace std;

struct ThreadArg{
    double** matrix1;
    double** matrix2;
    double** res_matrix;
    int start_row;
    int end_row;
    int matrix_size;
};

void delete_matrix(double** matrix, const int matrix_size)
{
    for(int i=0; i < matrix_size; ++i)
    {
        delete[] matrix[i];
    }

    delete[] matrix;
}

double** matrix_factory(const int matrix_size)
{
    double** matrix = new double*[matrix_size];

    for (int i = 0; i < matrix_size; ++i)
    {
        matrix[i] = new double[matrix_size];
    }

    return matrix;
}

double** read_matrix(const char* filename, int* size_buffer){
    int fd = open(filename, O_RDONLY);

    if (fd < 0)
    {
        cout << "open " << filename << " error" << endl;
    }

    off_t file_size = lseek(fd,0,SEEK_END);
    size_t num_elements = file_size / sizeof(double);
    *size_buffer = sqrt(num_elements);

    double** matrix = matrix_factory((*size_buffer));
    lseek(fd,0,SEEK_SET);

    for(int i = 0; i < (*size_buffer); ++i)
    {
        read(fd, matrix[i],(*size_buffer) * sizeof(double));
    }

    close(fd);
    return matrix;
}

double** multiply_matrices(double** matrix1, double** matrix2, const int matrix_size)
{
    double** res_matrix = matrix_factory(matrix_size);
    auto start = chrono::steady_clock::now();

    for (int i = 0; i < matrix_size; ++i)
    {
        for (int j = 0; j < matrix_size; ++j)
        {
            res_matrix[i][j] = 0;
            for (int k = 0; k < matrix_size; ++k)
            {
                res_matrix[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }

    auto end = chrono::steady_clock::now();
    chrono::duration<double> time = end - start;
    cout << "1 thread time: " << time.count() * 1000 << " ms" << endl;

    return res_matrix;
}

void* thread_multiply(void* tmp){
    ThreadArg* arg = (ThreadArg*)tmp;

    for (int i = arg->start_row; i< arg->end_row; ++i)
    {
        for (int j = 0; j < arg->matrix_size; ++j)
        {
            arg->res_matrix[i][j] = 0;
            for (int k = 0; k < arg->matrix_size; ++k)
            {
                arg->res_matrix[i][j] += arg->matrix1[i][k] * arg->matrix2[k][j];
            }
        }
    }

    return nullptr;
}

double** multiply_matrices_parallel(const int thread_num, double** matrix1,  double** matrix2, const int matrix_size)
{
    cout << "\nThreads num " << thread_num << " start" << endl;

    vector<pthread_t > threads;
    double** res_matrix = matrix_factory(matrix_size);
    int rows = matrix_size / thread_num;

    auto start = chrono::steady_clock::now();
    for(int i = 0; i < thread_num; ++i)
    {
        auto arg = new ThreadArg{matrix1, matrix2, res_matrix, i * rows, (i + 1) * rows, matrix_size};

        if (i == thread_num - 1)
        {
            arg->end_row = matrix_size;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, thread_multiply, arg);
        threads.push_back(tid);
    }

    for(size_t i = 0; i < thread_num; ++i)
    {
        pthread_join(threads[i], NULL);
    }

    auto end = chrono::steady_clock::now();
    chrono::duration<double> time = end - start;
    cout << thread_num << " threads time: " << time.count() * 1000 << " ms" << endl;
    return res_matrix;
}

bool check_multiply(double** matrix1, double** matrix2, const int matrix_size)
{
    for(int i = 0; i < matrix_size; ++i)
    {
        for(int j = 0; j < matrix_size; j++)
        {
            if(matrix1[i][j] != matrix2[i][j]) return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    int size1 = 0;
    int size2 = 0;
    double** matrix1 = read_matrix(argv[1], &size1);
    double** matrix2 = read_matrix(argv[2], &size2);

    cout << "Matrix 1 size = " << size1 << endl;
    cout << "Matrix 2 size = " << size2 << endl;

    if (size1 != size2) {
        cout << "Matrices size is not equal!" << endl;
        delete_matrix(matrix1, size1);
        delete_matrix(matrix2, size2);
        return 0;
    }

    double** consistenly_result_matrix = multiply_matrices(matrix1, matrix2, size1);

    for (int threads_num = 2; threads_num <= 6; threads_num += 2) {
        cout << endl << "======================================";
        double** parallel_result_matrix = multiply_matrices_parallel(threads_num, matrix1, matrix2, size1);
        if (check_multiply(consistenly_result_matrix, parallel_result_matrix, size1)) {
            cout << "\u2705 Correct \u2705";
        }
        else {
            cout << "\u274c ERROR \u274c";
        }
        cout << endl << "======================================" << endl;
        delete_matrix(parallel_result_matrix, size1);
    }


    delete_matrix(matrix1, size1);
    delete_matrix(matrix2, size2);
    delete_matrix(consistenly_result_matrix, size1);
    return 0;
}