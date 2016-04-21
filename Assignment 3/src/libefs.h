#include "efs.h"

/* FILE MODES for opening a file */
enum
{
    MODE_NORMAL=0, // Opens a file for read/write if it exists
    MODE_CREATE=1, // Create a new file and open it for read/write if it doesn't exist
    MODE_READ_ONLY=2, // Opens a file in Read Only Mode if it exists
};

// Additional Error Codes
//
enum {
    LIBFS_TOO_MANY_OPEN_FILES = 13371337,
    LIBFS_INDEX_ERROR = 77317731,
    LIBFS_READ_ONLY = 59595959
};



/* Open File Table structure. Feel free to modify */
typedef struct oft
{
    char filename[MAX_FNAME_LEN]; // Because the efs API doesn't expose _directory or accept INDEXES!!!!!!! Why the repeated calls to findFile??
    unsigned int free; // Marks if the slot is free. 0 for yes, 1 for no.
    unsigned char openMode; // Mode selected
    unsigned int blockSize; // Size of each block
    unsigned long inode; // Inode pointer
    unsigned long *inodeBuffer; // Inode buffer
    char *buffer; // Data buffer
    unsigned int writePtr; // Buffer index for writing data
    unsigned int readPtr; // Buffer index for reading data
    unsigned long filePtr; // File pointer. Points relative to ALL data in a file, not just the current buffer
} TOpenFile;

// Mounts a paritition given in fsPartitionName. Must be called before all
// other functions
void initFS(const char *fsPartitionName, const char *fsPassword);

// Opens a file in the partition. Depending on mode, a new file may be created
// if it doesn't exist, or we may get FS_FILE_NOT_FOUND in _result. See the enum above for valid modes.
// Return -1 if file open fails for some reason. E.g. file not found when mode is MODE_NORMAL, or
// disk is full when mode is MODE_CREATE, etc.

int openFile(const char *filename, unsigned char mode);

// Write data to the file. File must be opened in MODE_NORMAL or MODE_CREATE modes. Does nothing
// if file is opened in MODE_READ_ONLY mode.
// dataSize = size of each data unit. Use sizeof(.) to determine.
// dataCount = # of data units to write.
// Note dataSize * dataCount can exceed the size of one block.
// Returns the number of bytes successfully written
unsigned int writeFile(int fp, void *buffer, unsigned int dataSize, unsigned int dataCount);

// Flush the file data to the disk. Writes all data buffers, updates directory,
// free list and inode for this file.
void flushFile(int fp);

// Read data from the file.
// dataSize = size of each data unit. Use sizeof(.) to determine.
// dataCount = # of data units to write.
// Note dataSize * dataCount can exceed the size of one block.
// Returns number of bytes successfully read.
unsigned int readFile(int fp, void *buffer, unsigned int dataSize, unsigned int dataCount);

// Delete the file. Read-only flag (bit 2 of the attr field) in directory listing must not be set. 
// See TDirectory structure.
void delFile(const char *filename);

// Close a file. Flushes all data buffers, updates inode, directory, etc.
void closeFile(int fp);

// Unmount file system.
void closeFS();

