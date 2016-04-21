#include "libefs.h"

#define DEBUG 0
#define CHUNKSIZE 1024

int main(int ac, char **av)
{
    if (ac != 3)
    {
        printf("\nUsage: %s <file to check in> <password>\n\n", av[0]);
        return -1;
    }

    if (strlen(av[2]) > MAX_PWD_LEN) {
        puts("Error: too long password");
        return -1;
    }

    // Open the file to read from.
    FILE *fp = fopen(av[1], "r");

    if (fp == NULL) {
        puts("Error: no such file");
        return -1;
    }

    // Initialise the file system
    initFS("part.dsk", av[2]);

    // Open the file to write to
    int file_index = openFile(av[1], MODE_CREATE);

    // Check the return code.
    if (file_index == -1) {
        if (_result == FS_DUPLICATE_FILE) {
            puts("Error: DUPLICATE FILE");
        }
        else {
            puts("Error!");
        }
        closeFS();
        return 1;
    }

    unsigned int bytes_pending;
    unsigned long total_wrote = 0;
    char * chunk = new char[CHUNKSIZE](); // Write one block at a time
    do {
        bytes_pending = fread(chunk, sizeof(char), CHUNKSIZE, fp);
        writeFile(file_index, chunk, sizeof(char), bytes_pending);
        total_wrote += bytes_pending;
    } while (bytes_pending != 0);
    delete[] chunk;

    // Close the input file
    fclose(fp);

    // Close the open file
    closeFile(file_index);

    // Teardown the file system.
    closeFS();

    #if DEBUG == 1
        printf("File Index: %d\n", file_index);
        printf("Total Written: %lu\n", total_wrote);
    #endif

    return 0;
}
