#include "efs.h"

TFileSystemStruct _fsDescriptor;
TDirectory *_directory = NULL;
char *_bitmap = NULL;
FILE *_fsfp;

char _password[MAX_PWD_LEN];
char *_encBuffer = NULL;

unsigned long _result;

/*
   Private File System functions to load various structures like the file system parameters (descriptor),
   directory, free list bitmap and inode table

   All functions require you to first call mountfs
*/


// Encrypt/decrypt using simple XOR cipher
void encdec(char *targetBuffer, const char *message, unsigned int len, const char *key, unsigned int keyLen)
{
  unsigned long keyNdx = 0;

  for(unsigned int i=0; i<len; i++)
  {
    targetBuffer[i] = message[i] ^ key[keyNdx];
    keyNdx = i % keyLen;
  }
}

// Load file system parameters.

void loadFSDescriptor()
{
  // Seek to start
  fseek(_fsfp, 0, SEEK_SET);
  fread(&_fsDescriptor, sizeof(TFileSystemStruct), 1, _fsfp);
}

// Load directory
void loadDirectory()
{
  // Seek to the right place
  fseek(_fsfp, _fsDescriptor.dirByteIndex, SEEK_SET);

  if(_directory == NULL)
    _directory = (TDirectory *) calloc(sizeof(TDirectory), _fsDescriptor.maxFiles);

  fread(_directory, sizeof(TDirectory), _fsDescriptor.maxFiles, _fsfp);
}

// Write directory
void storeDirectory()
{
  // Seek to directory place
  fseek(_fsfp, _fsDescriptor.dirByteIndex, SEEK_SET);
  fwrite(_directory, sizeof(TDirectory), _fsDescriptor.maxFiles, _fsfp);
}

// Load free list bitmap
void loadBitmap()
{
  fseek(_fsfp, _fsDescriptor.bitmapByteIndex, SEEK_SET);

  if(_bitmap == NULL)
  {
    _bitmap = (char *) calloc(sizeof(char), _fsDescriptor.bitmapLen);

    if(_bitmap == NULL)
      printf("Bitmap allocation failed\n");
  }

  fread(_bitmap, sizeof(char), _fsDescriptor.bitmapLen, _fsfp);
}

// Store free list bitmap
void storeBitmap()
{
  fseek(_fsfp, _fsDescriptor.bitmapByteIndex, SEEK_SET);
  fwrite(_bitmap, sizeof(char), _fsDescriptor.bitmapLen, _fsfp);
}

// Calculate byte offset for a particular block number
unsigned long locateDataBlock(unsigned long blockNum)
{
  return _fsDescriptor.dataByteIndex + blockNum * _fsDescriptor.blockSize;
}


/*

   Public routines: Use these routines to implement your libraries

   */

/*

   Mount and unmount file system

   */

// Mount the file system. File system is stored on disk in "filename"
void mountFS(const char *filename, const char *password)
{
  _directory=NULL;
  _bitmap=NULL;
  _result = FS_OK;
  _encBuffer=NULL;

  strncpy(_password, password, MAX_PWD_LEN);

  _fsfp = fopen(filename, "r+");

  if(!_fsfp)
  {
    fprintf(stderr, "Unable to open partition file.\n");
    _result = FS_ERROR;
    exit(-1);
  }

  // Load descriptor
  loadFSDescriptor();

  if(_encBuffer == NULL)
    _encBuffer = (char *) calloc(sizeof(char), _fsDescriptor.blockSize);

  // Load directory
  loadDirectory();


  // load bitmap
  loadBitmap();

  _result = FS_OK;
}

// Unmount the file system
void unmountFS()
{
  storeDirectory();
  storeBitmap();
  fclose(_fsfp);

  if(_directory != NULL)
  {
    free(_directory);
    _directory = NULL;
  }

  if(_bitmap != NULL)
  {
    free(_bitmap);
    _bitmap = NULL;
  }

  if(_encBuffer != NULL)
  {
    free(_encBuffer);
    _encBuffer = NULL;
  }
}

// Return FS information
TFileSystemStruct *getFSInfo()
{
  return &_fsDescriptor;
}
/*

   Directory Management

   */


// Return index to next free directory entry
unsigned int getFreeDirectory()
{
  unsigned int i;

  for(i=0; i<_fsDescriptor.maxFiles; i++)
    if(!(_directory[i].attr & 0b1))
    {
      _result = FS_OK;
      return i;
    }

  _result = FS_DIR_FULL;
  return FS_DIR_FULL;
}

// Get length of file
unsigned long getFileLength(const char *filename)
{
	unsigned int ndx = findFile(filename);
	if(ndx != FS_FILE_NOT_FOUND)
	{
		_result = FS_OK;
		return _directory[ndx].length;
	}

	_result = ndx;
	return -1;
}

// Get inode for filename
unsigned long getInodeForFile(const char *filename)
{
  unsigned int ndx = findFile(filename);

  if(ndx != FS_FILE_NOT_FOUND)
  {
    _result = FS_OK;
    return _directory[ndx].inode;
  }

  _result = FS_FILE_NOT_FOUND;
  return _result;
}

// Make new directory entry
unsigned int makeDirectoryEntry(const char *filename, unsigned int attr, unsigned long len)
{
  unsigned int ndx = findFile(filename);

  if(ndx != FS_FILE_NOT_FOUND)
  {
    _result = FS_DUPLICATE_FILE;
    return _result;
  }

  ndx = getFreeDirectory();
  if(ndx != FS_DIR_FULL)
  {
    strncpy(_directory[ndx].filename, filename, MAX_FNAME_LEN);

    // Mark directory entry as taken
    _directory[ndx].attr = attr |= 0b1;
    _directory[ndx].length=len;
    _directory[ndx].inode = ndx;
    _result = FS_OK;
  }

  return ndx;
}

// Modify directory entry
unsigned int updateDirectoryFileLength(const char *filename, unsigned long len)
{
  unsigned int ndx = findFile(filename);

  if(ndx != FS_FILE_NOT_FOUND)
    _directory[ndx].length = len;

  return ndx;
}

// Remove directory entry
unsigned int delDirectoryEntry(const char *filename)
{
  unsigned int ndx = findFile(filename);

  if(ndx != FS_FILE_NOT_FOUND)
  {
    strcpy(_directory[ndx].filename, "nofile.dat");
    _directory[ndx].attr &= ~0b1;
  }

  return ndx;
}


// Search directory for file
unsigned int findFile(const char *filename)
{
  unsigned int i;

  for(i=0; i<_fsDescriptor.maxFiles; i++)
    if(!strcmp(_directory[i].filename, filename))
    {
      _result = FS_OK;
      return i;
    }

  _result = FS_FILE_NOT_FOUND;
  return FS_FILE_NOT_FOUND;
}

// Change attribute for file
void setattr(const char *filename, unsigned int attr)
{
  unsigned int ndx = findFile(filename);

  if(ndx != FS_FILE_NOT_FOUND)
    _directory[ndx].attr = attr;
}

// Get attribute for a file
unsigned int getattr(const char *filename)
{
  unsigned int ndx = findFile(filename);

  if(ndx != FS_FILE_NOT_FOUND)
    return _directory[ndx].attr;
  else
    return ndx;
}

void updateDirectory()
{
	storeDirectory();
}

/*

   Free List Management

   */

// Scans the bitmap for a free block.
unsigned long findFreeBlock()
{
  unsigned long maxBlockNum = _fsDescriptor.bitmapLen * 8;
  char testFlag = 0x80;
  unsigned int byteNdx=0;
  char bitPos = 0;

  for(unsigned long i = 0; i<maxBlockNum; i++)
    if(_bitmap[byteNdx] & testFlag)
    {
      _result = FS_OK;
			return i+1;
    }
    else
    {
      bitPos = (bitPos + 1) % 8;
      if(bitPos)
        testFlag = testFlag >> 1;
      else
      {
        byteNdx++;
        testFlag = 0x80;
      }
    }

  // Return 999999999 as full flag
  _result = FS_FULL;
  return FS_FULL;
}

// Return the byte number and bit offset for a block within the free bitmap
// Used by markBlockBusy and markBlockFree
void findBlockIndex(unsigned long blockNum, unsigned int *byteNum, unsigned char *bitNum)
{
  *byteNum = blockNum / 8;
  *bitNum = blockNum % 8;
}

// Mark a block as being used.
void markBlockBusy(unsigned long blockNum)
{
  unsigned int byteNum;
  unsigned char bitNum;

  findBlockIndex(blockNum-1, &byteNum, &bitNum);

  unsigned char testFlag = 0x80;
  testFlag = testFlag >> bitNum;
  _bitmap[byteNum] &= ~testFlag;
}

// Mark a block as being unused and free
void markBlockFree(unsigned long blockNum)
{
  unsigned int byteNum;
  unsigned char bitNum;
  unsigned char testFlag = 0x80;

  findBlockIndex(blockNum-1, &byteNum, &bitNum);
  testFlag = testFlag >> bitNum;
  _bitmap[byteNum] |= testFlag;
}

// Update the free list
void updateFreeList()
{
	storeBitmap();
}

/*
   inode Management

   */

// Allocate an inode buffer for a single inode
unsigned long *makeInodeBuffer()
{
	unsigned long *buffer = (unsigned long *) calloc(sizeof(unsigned long), _fsDescriptor.numInodeEntries);
	return buffer;
}

// Load a particular inode
void loadInode(unsigned long *inode, unsigned int inodeNumber)
{
  unsigned long inodeIndex = _fsDescriptor.inodeByteIndex + inodeNumber * _fsDescriptor.blockSize;
  fseek(_fsfp, inodeIndex, SEEK_SET);
  fread(inode, sizeof(unsigned long), _fsDescriptor.numInodeEntries, _fsfp);
}

// Write an inode
void saveInode(unsigned long *inode, unsigned int inodeNumber)
{
  unsigned long inodeIndex = _fsDescriptor.inodeByteIndex + inodeNumber * _fsDescriptor.blockSize;
  fseek(_fsfp, inodeIndex, SEEK_SET);
  fwrite(inode, sizeof(unsigned long), _fsDescriptor.numInodeEntries, _fsfp);
}

// Set block number in an inode given a byte offset
void setBlockNumInInode(unsigned long *inode, unsigned long byteNumber, unsigned long blockNumber)
{
  unsigned int index = byteNumber / _fsDescriptor.blockSize;
  inode[index] = blockNumber;
}

// Get a block number from inode given a byte offset
unsigned long returnBlockNumFromInode(unsigned long *inode, unsigned long byteNumber)
{
  unsigned int index = byteNumber / _fsDescriptor.blockSize;
  return inode[index];
}

/*

   Read/write data block. Block numbers start from 1, not 0.

   */

// Create a data buffer
char *makeDataBuffer()
{
	char *buffer = (char *) calloc(sizeof(char), _fsDescriptor.blockSize);
	return buffer;
}

// Read a data block from disk
void readBlock(char *buffer, unsigned long blockNum)
{
  unsigned long byteIndex = locateDataBlock(blockNum-1);
  fseek(_fsfp, byteIndex, SEEK_SET);
  fread(_encBuffer, sizeof(char), _fsDescriptor.blockSize, _fsfp);
  encdec(buffer, _encBuffer, _fsDescriptor.blockSize, _password, strlen(_password));
}

// Write a data block to disk
void writeBlock(char *buffer, unsigned long blockNum)
{
  unsigned long byteIndex = locateDataBlock(blockNum-1);
  fseek(_fsfp, byteIndex, SEEK_SET);
  encdec(_encBuffer, buffer, _fsDescriptor.blockSize, _password, strlen(_password));
  fwrite(_encBuffer, sizeof(char), _fsDescriptor.blockSize, _fsfp);
}
