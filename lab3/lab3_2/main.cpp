#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <chrono>
#include <vector>
#include <chrono>
#include <cstring>
#include <pthread.h>

using namespace std;

size_t thread_num = 3;
pthread_barrier_t  barrier;
volatile int  result_index = -1;
vector<int> finished;

struct ThreadArg{
    int* array;
    int number;
    int start_index;
    int end_index;
    int array_size;
};

int* create_filled_array(const int array_size){
    int* array = new int[array_size];
    for(int i = 0; i < array_size; i++){
        array[i] = rand() % 20;
    }
    array[1] = 21;
    return array;
}

void print_array(int* array, const int array_size){
    cout << "Array:" << endl;
    for (int i = 0; i < array_size; i++) {
        cout << array[i] << ' ';
    }
    cout << endl;
}

void delete_array(int* array)
{
    delete[]array;
}

int* array_fabric(const int array_size)
{
    int* array = new int[array_size];
    return array;
}

int* read_array(const char* filename, int* array_size)
{
    int fd = open(filename, O_RDONLY);

    if (fd < 0)
    {
        cout << "open " << filename << " error" << endl;
    }

    off_t file_size = lseek(fd,0,SEEK_END);
    *array_size = file_size / sizeof(int);

    int* array = array_fabric((*array_size));
    lseek(fd,0,SEEK_SET);

    read(fd, array, (*array_size) * sizeof (int));
    array[999998] = 1000;
    close(fd);
    return array;
}

int standart_search(int* array, const int number2search, const int array_size)
{

    auto start =  chrono::steady_clock::now();
    for(int i = 0; i < array_size; ++i)
    {
        if(array[i] == number2search)
        {
            auto end =  chrono::steady_clock::now();
            chrono::duration<double> time = end - start;
            cout << "1 thread time: " << time.count() * 1000 << " ms" <<  endl;
            return i;
        }
    }
    auto end =  chrono::steady_clock::now();
    chrono::duration<double> time = end - start;
    cout << "1 thread time: " << time.count() * 1000  << " ms" <<  endl;
    return -1;
}

void* thread_search(void* tmp)
{
    ThreadArg* arg = (ThreadArg*)tmp;
    int local_index = -1;

    for (int i = arg->start_index; i < arg->end_index; ++i)
    {
        if(arg->array[i] == arg->number)
        {
            local_index = i;
            break;
        }
    }

    cout << "++";
    cout << local_index << endl;
    finished.push_back(local_index);

    if(pthread_barrier_wait(&barrier) == PTHREAD_BARRIER_SERIAL_THREAD) {
        cout << "enter";
        int tmp = arg->array_size;
        for (int i = 0; i < finished.size(); i++){
            if(finished[i] < tmp && finished[i] != -1) tmp = finished[i];
        }
        result_index = tmp;
    }

    delete arg;
    return nullptr;
}

int search_parallel(const int threads_num, int* array, const int number2search, const int array_size)
{
    vector<pthread_t> threads;

    int slice = array_size / threads_num;
    int start_index = 0;
    int end_index = 0;

    auto start =  chrono::steady_clock::now();

    for(int i = 0; i < threads_num; ++i)
    {
        auto arg = new ThreadArg{array, number2search ,i * slice, (i + 1) * slice, array_size};
        if (i == thread_num - 1)
        {
            arg->end_index = array_size;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, thread_search, arg);
        threads.push_back(tid);
        start_index += slice;
    }

    pthread_barrier_wait(&barrier);

    auto end =  chrono::steady_clock::now();
    chrono::duration<double> time = end - start;
    cout << threads_num << " threads time: " << time.count() * 1000  << " ms" <<  endl;
    return result_index;
}

int main(int argc, char* argv[]) {
    int size = 0;
    int* array;
    int number2serch = 1000;

    if (strcmp(argv[1], "size") == 0) {
        size = atoi(argv[2]);
        array = create_filled_array(size);
        number2serch = rand() % 10;
    }
    else {
        array = read_array(argv[1], &size);
    }
    // print_array(array, size);

    cout  << endl << "Array size = " << size <<  endl;
    cout  << "Number to search = " << number2serch << endl;


    cout << endl << "========================================\nStart 1 thread" << endl;
    int result = standart_search(array, number2serch, size);
    if (result != -1) {
        cout << "\u2705 Result with 1 trhead search: " << result  << " \u2705" <<  endl;
    }
    else {
        cout << "\u274c NOT FOUND with 1 thread \u274c" << endl;
    }
    cout << "========================================" << endl;


    for (int threads_num = 2; threads_num <= 4; threads_num += 2) {
        cout << endl << "========================================\nStart " << threads_num << " thread" << endl;
        pthread_barrier_init(&barrier, NULL, threads_num + 1);
        result = search_parallel(threads_num, array, number2serch, size);
        if (result != -1) {
            cout << "\u2705 Result with " << threads_num << " trheads search: " << result << " \u2705" <<  endl;
        }
        else {
            cout << "\u274c NOT FOUND with " << threads_num << " trheads \u274c" << endl;
        }
        pthread_barrier_destroy(&barrier);
        cout << "========================================" << endl;
    }



    delete_array(array);
    return 0;
}