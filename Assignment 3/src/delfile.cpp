#include "libefs.h"

int main(int ac, char **av)
{
    if(ac != 2)
    {
        printf("\nUsage: %s <file to delete>\n\n", av[0]);
        return -1;
    }

    initFS("part.dsk", "\x00");

    delFile(av[1]);
    if (_result == FS_FILE_NOT_FOUND) {
        puts("Error: File not found");
    }
    else if (_result == LIBFS_READ_ONLY) {
        puts("Error: File read only");
    }

    closeFS();

    return 0;
}
