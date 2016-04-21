#include "efs.h"

int main(int ac, char **av)
{
	if(ac != 3)
	{
		printf("\nUsage: %s <filename> <password>\n\n", av[0]);
		exit(-1);
	}

	// Mount the file system
	mountFS("part.dsk", av[2]);

	// Get the attributes
	TFileSystemStruct *fs = getFSInfo();

	// Allocate the inode and buffer
	unsigned long *inode = makeInodeBuffer();
	char *buffer = makeDataBuffer();

	// Search the directory for the file
	unsigned int fileNdx = findFile(av[1]);

	if(fileNdx == FS_FILE_NOT_FOUND)
	{
		printf("Cannot find encrypted file %s\n", av[1]);
		exit(-1);
	}

	// Get file length
	unsigned int len = getFileLength(av[1]);

	// Load the inode
	loadInode(inode, fileNdx);

	// Get the block number
	unsigned long blockNum = inode[0];

	// Read the block
	readBlock(buffer, blockNum);

	// Unmount the file system
	unmountFS();

	// Open output file
	FILE *fp = fopen(av[1], "w");

	// Write the data
	fwrite(buffer, sizeof(char), len, fp);

	// Close the file
	fclose(fp);
	
	free(inode);
	free(buffer);
	return 0;
}

