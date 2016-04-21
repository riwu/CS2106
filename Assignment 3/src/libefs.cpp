#include "libefs.h"

#define MAX_OPEN_FILES 255

// FS Descriptor
TFileSystemStruct *_fs;

// Open File Table
TOpenFile *_oft;

// Free open file slots
int _freeOft = MAX_OPEN_FILES;

// Mounts a paritition given in fsPartitionName. Must be called before all
// other functions
void initFS(const char *fsPartitionName, const char *fsPassword)
{
    // Mount the partition first
    mountFS(fsPartitionName, fsPassword);

    // Initialise the file system descriptor
    _fs = getFSInfo();

    // Allocate memory for the Open File Table
    _oft = new TOpenFile[MAX_OPEN_FILES](); // Initialises to 0
}

// Opens a file in the partition. Depending on mode, a new file may be created
// if it doesn't exist, or we may get FS_FILE_NOT_FOUND in _result. See the enum above for valid modes.
// Return -1 if file open fails for some reason. E.g. file not found when mode is MODE_NORMAL, or
// disk is full when mode is MODE_CREATE, etc.

int openFile(const char *filename, unsigned char mode)
{
    // Check that we have free slots
    if (_freeOft == 0) {
        _result = LIBFS_TOO_MANY_OPEN_FILES;
        return -1;
    }

    // Search for a free slot.
    int index = -1;
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (_oft[i].free == 0) {
            index = i;
            break;
        }
    }

    // If it failed for some reason, return -1.
    if (index == -1) {
        _result = LIBFS_INDEX_ERROR;
        return -1;
    }

    unsigned int dir_entry = -1;
    unsigned long *inode_buffer = NULL;

    if (mode == MODE_CREATE) {
        // Handle the MODE_CREATE option

        // Create a new file of 0 length with null attributes
        dir_entry = makeDirectoryEntry(filename, 0, 0);

        // Check for errors
        if (_result == FS_DUPLICATE_FILE || _result == FS_ERROR ||
            _result == FS_FULL || _result == FS_DIR_FULL) {
            return -1;
        }

        // Only allocate the inode buffer if the above passes.
        inode_buffer = makeInodeBuffer();

        // Allocate the first block for this new buffer
        unsigned long free_block = findFreeBlock();
        markBlockBusy(free_block);
        setBlockNumInInode(inode_buffer, 0, free_block);
    } else {
        // Handle the MODE_NORMAL and MODE_READ_ONLY options

        // Search for the directory entry matching the filename
        dir_entry = findFile(filename);

        // Check that the file exists
        if (_result == FS_FILE_NOT_FOUND || _result == FS_ERROR) {
            return -1;
        }

        // Only allocate and load the inode buffer if the above passes.
        inode_buffer = makeInodeBuffer();
        loadInode(inode_buffer, (unsigned long) dir_entry);
    }

    // Initialise the OFT
    strncpy(_oft[index].filename, filename, MAX_FNAME_LEN);
    _oft[index].openMode = mode;
    _oft[index].blockSize = _fs->blockSize;
    _oft[index].inode = (unsigned long) dir_entry;
    _oft[index].inodeBuffer = inode_buffer; // Needs to be freed
    _oft[index].buffer = makeDataBuffer(); // Needs to be freed
    _oft[index].writePtr = 0;
    _oft[index].readPtr = 0;
    _oft[index].filePtr = 0;

    // Mark the slot as used.
    _oft[index].free = 1;
    --_freeOft;

    // Read the first block into the buffer.
    unsigned long block_number = inode_buffer[0];
    readBlock(_oft[index].buffer, block_number);

    return index;
}

// Write data to the file. File must be opened in MODE_NORMAL or MODE_CREATE modes. Does nothing
// if file is opened in MODE_READ_ONLY mode.
// Returns the number of bytes successfully written
unsigned int writeFile(int fp, void *buffer, unsigned int dataSize, unsigned int dataCount) {
    if (_oft[fp].openMode == MODE_READ_ONLY) {
        return 0;
    }

    unsigned int total_wrote = 0;
    unsigned long block_number;
    unsigned long data_len = dataCount * dataSize;

    // Increase the length of the file
    unsigned long file_len = getFileLength(_oft[fp].filename);
    updateDirectoryFileLength(_oft[fp].filename, file_len + data_len);

    // Total max bytes written should be dataCount * dataSize
    for (int i = 0; i < data_len ; i++) {
        // If the writePtr == blocksize, flush, and load a new block
        if (_oft[fp].writePtr == _fs->blockSize) {
            // Flush the current block
            block_number = returnBlockNumFromInode(_oft[fp].inodeBuffer,
                                                   _oft[fp].filePtr - 1);
            writeBlock(_oft[fp].buffer, block_number);

            // Load a new block
            block_number = findFreeBlock();
            markBlockBusy(block_number);
            setBlockNumInInode(_oft[fp].inodeBuffer, _oft[fp].filePtr,
                               block_number);

            // Zero out the buffer
            memset(_oft[fp].buffer, 0, _fs->blockSize);

            _oft[fp].writePtr = 0;
        }

        // Byte by byte copy
        unsigned int cptr = _oft[fp].writePtr;
        memcpy(_oft[fp].buffer + cptr, ((char * ) buffer) + i, sizeof(char));
        ++_oft[fp].writePtr;
        ++_oft[fp].filePtr;
        ++total_wrote;
    }

    return total_wrote;
}

// Flush the file data to the disk. Writes all data buffers, updates directory,
// free list and inode for this file.
void flushFile(int fp)
{
    // Flush buffers
    unsigned long block_number = returnBlockNumFromInode(_oft[fp].inodeBuffer,
                                                         _oft[fp].filePtr);
    writeBlock(_oft[fp].buffer, block_number);

    // Update the partition
    saveInode(_oft[fp].inodeBuffer, _oft[fp].inode);
    updateDirectory();
    updateFreeList();
}

// Read data from the file.
// Returns the number of bytes successfully read
unsigned int readFile(int fp, void *buffer, unsigned int dataSize, unsigned int dataCount) {
    unsigned int total_read = 0;
    unsigned long block_number;

    unsigned long file_len = getFileLength(_oft[fp].filename);

    // Total max bytes read should be dataCount * dataSize
    for (int i = 0; i < (dataCount * dataSize) ; i++) {
        // If we attempt to read past the end of the file, stop immediately.
        if (_oft[fp].filePtr == file_len) {
            break;
        }

        // If the readPtr == blocksize, load a new block
        if (_oft[fp].readPtr == _fs->blockSize) {
            block_number = returnBlockNumFromInode(_oft[fp].inodeBuffer,
                                                   _oft[fp].filePtr);
            readBlock(_oft[fp].buffer, block_number);
            _oft[fp].readPtr = 0;
        }

        // Byte by byte copy
        unsigned int cptr = _oft[fp].readPtr;
        memcpy(((char * ) buffer) + i, _oft[fp].buffer + cptr, sizeof(char));
        ++_oft[fp].readPtr;
        ++_oft[fp].filePtr;
        ++total_read;
    }
    return total_read;
}

// Delete the file. Read-only flag (bit 2 of the attr field) in directory listing must not be set.
// See TDirectory structure.
void delFile(const char *filename){
    unsigned int attr = getattr(filename);
    if (attr == FS_FILE_NOT_FOUND) {
        return;
    }
    else if (attr & 0b100) {
        _result = LIBFS_READ_ONLY;
        return;
    }

    unsigned int file_index = findFile(filename);
    unsigned long * inode_buffer = makeInodeBuffer();
    loadInode(inode_buffer, (unsigned long) file_index);

    // Mark all the block entries in the inode as free.
    for (int i = 0; i < _fs->numInodeEntries; i++) {
        if (inode_buffer[i] != 0) {
            markBlockFree(inode_buffer[i]);
        }
    }

    // Update the Free List
    updateFreeList();

    // Delete the directory entry.
    delDirectoryEntry(filename);

    // Update the directory
    updateDirectory();

    // Free the inode buffer.
    free(inode_buffer);
}

// Close a file. Flushes all data buffers, updates inode, directory, etc.
void closeFile(int fp) {
    // Flush file
    flushFile(fp);

    // Free the OFT entry
    free(_oft[fp].inodeBuffer);
    free(_oft[fp].buffer);
    _oft[fp].free = 0;
    ++_freeOft;
}

// Unmount file system.
void closeFS() {

    // Unmount the file system.
    unmountFS();

    // Free the Open File Table
    delete[] _oft;
}

