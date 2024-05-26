#include "check.h"
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <random>
#include <iostream>
#include <vector>

#define N 10

using namespace std;

volatile sig_atomic_t last_sig;
volatile sig_atomic_t sig_val;

pid_t child_p, parent_p;
bool flag;

sigset_t mask;
sigset_t set;

struct sigaction sig_action;

void rtsig_handler(int signo, siginfo_t* si, void* ctx) {
    last_sig = signo;
    sig_val = si->si_value.sival_int;
}

bool check_exist(vector<int> arr, int x)
{
    for (int i = 0; i < arr.size(); i++)
    {
        if (arr[i] == x) return true;
    }
    return false;
}

void my_sleep()
{
    sleep(1);
}

void guesser()
{
    sigset_t set;
    vector<int> arr;
    int x;
    int count = 0;
    sigemptyset(&set);

    sigsuspend(&set);

    if (child_p != 0)
        cout << "i'm geniy, i'm a parent" << endl;
    else
        cout << "i'm geniy, i'm a child" << endl;

    while(true)
    {
        if (last_sig == SIGUSR2)
        {
            x = rand() % (10 - 1 + 1) + 1;
            while(true)
            {
                if (check_exist(arr, x))
                    x = rand() % (N - 1 + 1) + 1;
                else
                    {
                        arr.push_back(x);
                        break;
                    }
            }
            cout << "My value = " << x << endl;
            count += 1;
            if (flag == true)
                sigqueue(parent_p, SIGUSR2, sigval{x});
            else
                sigqueue(child_p, SIGUSR2, sigval{x});
        }
        else if (last_sig == SIGUSR1)
        {
            cout << "I need " << count << " attempts to win" << endl;

            break;
        }
        sigsuspend(&set);
    }
    flag = !flag;
    arr.clear();
}

void game()
{
    sigset_t set;
    sigemptyset(&set);

    if (child_p != 0)
        cout << "I'm a parent ";
    else
        cout << "I'm a child ";
    int number = rand() % (N - 1 + 1) + 1;
    cout << "I'm wish a number = " << number << endl;


    if (child_p != 0) {
        check(sigqueue(child_p, SIGUSR2, sigval{-2}));
    }
    else {
        check(sigqueue(parent_p, SIGUSR2, sigval{-2}));

    }
    while(true)
    {
        if (sig_val == number) {
            if (flag == true) {
                sigqueue(child_p, SIGUSR1, sigval{-1});
            }
            else {
                check(sigqueue(parent_p, SIGUSR1, sigval{-1}));
            }
            cout << "YEEEES. YOU DID IT!!!" << endl;
            break;
        }
        else {
            cout << "You not guess." << endl;
            if (flag == true) {
                check(sigqueue(child_p, SIGUSR2, sigval{-2}));
            }
            else {
                check(sigqueue(parent_p, SIGUSR2, sigval{-2}));

            }
        }
        sigsuspend(&set);

        my_sleep();
    }
    flag = !flag;
}


int main()
{
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);

    parent_p = getpid();
    sig_action.sa_sigaction = rtsig_handler;
    sig_action.sa_flags = SA_SIGINFO;
    sig_action.sa_mask = mask;
    check(sigaction(SIGUSR1, &sig_action, NULL));
    check(sigaction(SIGUSR2, &sig_action, NULL));



    sigprocmask(SIG_BLOCK, &mask, NULL);

    child_p = fork();

    if (child_p != 0)
    {
        cout << "Child pid" << child_p << endl;
        cout << "Parent pid" << parent_p << endl;
    }

    flag = true;
    for (int i = 0; i < 2; i++)
    {
        if (child_p != 0)
        {
            cout << "\n\nGame № " << i + 1 << endl;
            srand(time(0));
            if (flag == true)
                game();
            else
                guesser();
        }
        else
        {
            srand(time(0) + 100);
            if (flag == true)
                guesser();
            else
                game();
        }
        my_sleep();
    }

    return 0;
}