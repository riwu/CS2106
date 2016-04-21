#include "libefs.h"

#define DEBUG 0
#define CHUNKSIZE 1024

int main(int ac, char **av)
{
    if (ac != 3)
    {
        printf("\nUsage: %s <file to check out> <password>\n\n", av[0]);
        return -1;
    }

    if (strlen(av[2]) > MAX_PWD_LEN) {
        puts("Error: too long password");
        return -1;
    }

    // Open the file to write to.
    FILE *fp = fopen(av[1], "w");

    if (fp == NULL) {
        puts("Error: no such file");
        return -1;
    }

    // Initialise the file system
    initFS("part.dsk", av[2]);

    // Open the file to read from.
    int file_index = openFile(av[1], MODE_READ_ONLY);

    // Check the return code.
    if (file_index == -1) {
        if (_result == FS_FILE_NOT_FOUND) {
            puts("Error: FILE NOT FOUND");
        }
        else {
            puts("Error!");
        }
        closeFS();
        return 1;
    }

    // Read the data from the EFS and write to the file on disk
    unsigned int bytes_read;
    unsigned long total_read = 0;
    char * chunk = new char[CHUNKSIZE]();
    do {
        bytes_read = readFile(file_index, chunk, sizeof(char), CHUNKSIZE);
        fwrite(chunk, sizeof(char), bytes_read, fp);
        total_read += bytes_read;
    } while (bytes_read != 0);
    delete[] chunk;

    // Close the output file
    fclose(fp);

    // Close the open file
    closeFile(file_index);

    // Teardown the file system.
    closeFS();

    #if DEBUG == 1
        printf("File Index: %d\n", file_index);
        printf("Total Read: %lu\n", total_read);
    #endif

    return 0;
}
