#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Maximum password length
#define MAX_PWD_LEN   32

// Maximum filename length
#define MAX_FNAME_LEN 32


enum
{
  FS_OK = 0,
  FS_ERROR = 1,
  FS_FULL = 99999999,
  FS_DIR_FULL = 8888888,
  FS_FILE_NOT_FOUND=7777777,
  FS_DUPLICATE_FILE=6666666
};

/*

   Data structure definitions for the file system

   */

typedef struct fs
{
  unsigned long fsSize; // File system size in megabytes
  unsigned int blockSize; // Size of blocks in bytes
  unsigned int maxFiles; // Maximum number of blocks

  /* Computed Values */
  unsigned int numBlocks; // Number of blocks. Computed.
  unsigned int numInodeEntries; // # of entries in inode block
  unsigned int bitmapLen; // Length of bitmap in bytes.
  unsigned int dirByteIndex; // Index to the file directory
  unsigned int bitmapByteIndex; // Index to the bitmap entry
  unsigned int inodeByteIndex; // Index to inode table
  unsigned int dataByteIndex; // Index to first data block
} TFileSystemStruct;

typedef struct dir
{
  char filename[MAX_FNAME_LEN];
  unsigned long length; // Length of file in bytes
  char attr;  // bit 0 clear - Free entry, bit 1 set - Subdirectory, bit 2 set - Read Only
  unsigned long inode;
} TDirectory;

extern unsigned long _result; // Result of file system operation

/*

   Public routines: Use these routines to implement your libraries

   */

/* 

   Mount and unmount file system

   */

// Mount the file system. File system is stored on disk in "filename", password to 
// encrypt decrypt given in password
void mountFS(const char *filename, const char *password);

// Unmount the file system
void unmountFS();

// Return FS information
TFileSystemStruct *getFSInfo();

/*

   Directory Management

   */


// Make new directory entry
unsigned int makeDirectoryEntry(const char *filename, unsigned int attr, unsigned long len);

// Modify directory entry
unsigned int updateDirectoryFileLength(const char *filename, unsigned long len);

// Remove directory entry
unsigned int delDirectoryEntry(const char *filename);

// Search directory for file
unsigned int findFile(const char *filename);

// Get inode for filename
unsigned long getInodeForFile(const char *filename);

// Get length of file
unsigned long getFileLength(const char *filename);

// Change attribute for file
void setattr(const char *filename, unsigned int attr);

// Get attribute for a file
unsigned int getattr(const char *filename);

// Write directory to partition
void updateDirectory();

/*

   Free List Management

   */

// Scans the bitmap for a free block. 
unsigned long findFreeBlock();

// Mark a block as being used.
void markBlockBusy(unsigned long blockNum);

// Mark a block as being unused and free
void markBlockFree(unsigned long blockNum);

// Update the free list
void updateFreeList();
/*
   inode Management

   */

// Allocate an inode buffer for a single inode
unsigned long *makeInodeBuffer();

// Load a particular inode
void loadInode(unsigned long *inode, unsigned int inodeNumber);

// Write an inode
void saveInode(unsigned long *inode, unsigned int inodeNumber);

// Set block number in an inode given a byte offset
void setBlockNumInInode(unsigned long *inode, unsigned long byteNumber, unsigned long blockNumber);

// Get a block number from inode given a byte offset
unsigned long returnBlockNumFromInode(unsigned long *inode, unsigned long byteNumber);

/*

   Read/write data block

   */

// Create a data buffer
char *makeDataBuffer();

// Read a data block from disk
void readBlock(char *buffer, unsigned long blockNum);

// Write a data block to disk
void writeBlock(char *buffer, unsigned long blockNum);
