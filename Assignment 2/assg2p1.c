#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int fd[2];
    char string[]="Hello child!";
    char buffer[80];
    int pnum=128, cnum;

    pipe(fd);

    if(fork())
    {
        close(fd[0]);
        write(fd[1], &pnum, sizeof(pnum));
        write(fd[1], string, strlen(string)+1);
    }
    else
    {
        close(fd[1]);
        read(fd[0], &cnum, sizeof(cnum));
        read(fd[0], buffer, sizeof(buffer));
        printf("Parent sent message: %s and %d\n", buffer, cnum);
    }
}
