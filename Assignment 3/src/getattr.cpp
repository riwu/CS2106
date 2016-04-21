#include "libefs.h"

int main(int ac, char **av)
{
    if(ac != 2)
    {
        printf("\nUsage: %s <file to check>\n", av[0]);
        printf("Prints: 'R' = Read only, 'W' = Read/Write\n\n");
        return -1;
    }

    initFS("part.dsk", "\x00");

    // Get the original file's attributes
    unsigned int file_attr = getattr(av[1]);

    if (file_attr == FS_FILE_NOT_FOUND) {
        puts("Error: file not found");
        closeFS();
        return -1;
    }

    int rw = file_attr & 0b100;

    if (rw) {
        puts("R");
    }
    else {
        puts("W");
    }

    closeFS();

    return 0;
}
