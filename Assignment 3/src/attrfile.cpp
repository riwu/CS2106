#include "libefs.h"

int main(int ac, char **av)
{
    if(ac != 3)
    {
        printf("\nUsage: %s <file to check set attrbute> <attribute>\n", av[0]);
        printf("Attribute: 'R' = Read only, 'W' = Read/Write\n\n");
        return -1;
    }

    // Parse the attribute argument
    int attb = 0;
    if (strcmp(av[2], "R") == 0) {
        attb = 1;
    }
    else if (strcmp(av[2], "W") == 0) {
        attb = 2;
    }

    if (!attb) {
        puts("Error: invalid attribute");
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

    // Mask the attribute
    if (attb == 2) {
        file_attr = file_attr & 0b11;
    } else {
        file_attr = file_attr | 0b100;
    }

    setattr(av[1], file_attr);

    closeFS();

    return 0;
}
