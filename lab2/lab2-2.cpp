#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

using namespace std;
int N = 0;

void beautiful_print(const int i, const bool is_child = false) {
    printf("\n\n=======================");
    if (!is_child) printf("\nNow PARENT puzzler\n");
    else printf("\nNow CHILD puzzler\n");
    printf("\nCurrent round: %d\n\n", i + 1);
}

int randomizer(const int max_value, const int min_value = 0) {
    return rand() % (max_value - min_value) + 1;
}

void puzzler(int id_w, int id_r, const int current_round, const bool is_child = false)
{
    beautiful_print(current_round, is_child);
    srand(time(NULL));
    int val = randomizer(N);
    printf("Puzzler wish number: %d\n", val);

    int x_read, buff, x_write;
    int count = 0;
    int tmp = 0;
    x_write = write(id_w, &tmp,sizeof(tmp));
    do
    {
        x_read = read(id_r,&buff, sizeof(int));
        if (x_read > 0)
        {
            if (buff != val)
            {
                ++count;
                x_write = write(id_w, &tmp, sizeof(tmp));
            }
            else
            {
                ++count;
                tmp = -1;
                x_write = write(id_w, &tmp, sizeof(tmp));
                break;
            }
        }
    } while (x_read);
    printf("\n=====> Guessed right in %d attempts\n\n", count);
}

void guesser(int id_w, int id_r)
{
    int x_write, x_read, buff;
    int val = 1;
    do {
        x_read = read(id_r, &buff, sizeof(int));
        if(x_read > 0)
        {
            if (buff == 0)
            {
                printf("Guesser think that number is %d\n", val);
                x_write = write(id_w, &val, sizeof(val));
                val++;
            }
            else return;
        }
    } while (x_read);
}

int main(int argc, char *argv[])
{
    if (argc != 2) exit(EXIT_FAILURE);
    N = atoi(argv[1]);

    if (N < 0) {
        N = randomizer(10);
        printf("\nNo no no %d is bad number\nOK, round count=%d\n", atoi(argv[1]), N);
    }

    int fd_parent[2];
    int fd_child[2];

    pipe(fd_parent);
    pipe(fd_child);

    pid_t pid = fork();
    //p = pid;
    for (int i = 0; i < N; ++i)
    {
        if (pid) puzzler(fd_parent[1], fd_child[0], i);
        else guesser(fd_child[1], fd_parent[0]);


        if (pid) guesser(fd_parent[1], fd_child[0]);
        else puzzler(fd_child[1], fd_parent[0], i, true);
    }
    close(fd_parent[0]);
    close(fd_child[0]);
    close(fd_parent[1]);
    close(fd_child[1]);

    return 0;
}