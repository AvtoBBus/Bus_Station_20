#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>


int main()
{
    const char * STARTWAY = "ROOT";
    mkdir(STARTWAY, S_IRWXU);
    chdir(STARTWAY);

    mkdir("a_1", S_IRWXU);
    mkdir("a_2", S_IRWXU);

    chdir("a_1");
    mkdir("b_0", S_IRWXU);

    chdir("../a_2");
    mkdir("b_1", S_IRWXU);

    chdir("../a_1/b_0");
    symlink("../../a_2/b_1", "c_0");

    chdir("../../a_2/b_1");
    int fd = open("c_1.bin", O_CREAT|O_RDWR|O_TRUNC, S_IRWXU);
    if (fd == -1)
    {
        perror("error open c_1.bin");
        return -1;
    }
    ftruncate(fd, 227);
    close(fd);

    fd = open("c_3.bin", O_CREAT|O_RDWR|O_TRUNC, S_IRWXU);
    close(fd);

    chdir("../../a_2");
    mkdir("b_2", S_IRWXU);
    chdir("b_2");
    
    link("../b_1/c_3.bin", "c_2.bin");
    
    chdir("../../");

    symlink("a_2/b_1/c_3.bin", "a_0.bin");

    system("tree");
    return 0;
}