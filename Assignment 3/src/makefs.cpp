#include "efs.h"

char *bitmap;
TDirectory *directory;
unsigned long **inodeTable;

int main(int ac, char **av)
{
  if(ac<2)
  {
    fprintf(stderr, "\nUsage: %s <config filename>\n\n", av[0]);
    return -1;
  }

  TFileSystemStruct filesys;
  int i;

  FILE *fp;

  fp = fopen(av[1], "r");
  if(fp == NULL)
  {
    fprintf(stderr, "Unable to open config file.\n");
    return -1;
  }

  /* Read the config file */
  char partName[32];
  fscanf(fp, "%s\n", partName);
  fscanf(fp, "%lu\n", &filesys.fsSize);
  fscanf(fp, "%d\n", &filesys.blockSize);
  fscanf(fp, "%d\n", &filesys.maxFiles);
  fclose(fp);

  directory = (TDirectory *) calloc(sizeof(TDirectory), filesys.maxFiles);

  for(i = 0; i<filesys.maxFiles; i++)
  {
    strcpy(directory[i].filename, "nofile.dat");
    directory[i].attr=0;
  }

  filesys.fsSize  = filesys.fsSize * 1024 * 1024;

  // Start calculating all the byte indexes
  filesys.numBlocks = filesys.fsSize / filesys.blockSize;

  // Allocate the bitmap
  filesys.bitmapLen = ceil(filesys.numBlocks / 8);
  bitmap = (char *) calloc(sizeof(char), filesys.bitmapLen);
  memset(bitmap, 0xff, filesys.bitmapLen);

  // # of entries per inode block
  filesys.numInodeEntries = filesys.blockSize / sizeof(unsigned long);

  // Allocate the inode table
  inodeTable = (unsigned long **) calloc(sizeof(unsigned long), filesys.maxFiles);

  for(i=0; i<filesys.maxFiles; i++)
    inodeTable[i] = (unsigned long *) calloc(sizeof(unsigned long), filesys.numInodeEntries);

  // Directory begins after the metadata
  filesys.dirByteIndex = sizeof(TFileSystemStruct);

  // Bitmap begins after the directory, which is size of each entry * maxfiles
  filesys.bitmapByteIndex = filesys.dirByteIndex + sizeof(TDirectory) * filesys.maxFiles;

  // inode table begins after bitmaps
  filesys.inodeByteIndex = filesys.bitmapByteIndex + filesys.bitmapLen;

  // Data table begins after inode table. There is one inode per file, and each inode is one block
  filesys.dataByteIndex = filesys.inodeByteIndex + filesys.blockSize * filesys.maxFiles;

  // Usable data space
  unsigned long usableSpace = filesys.fsSize - filesys.dataByteIndex + 1;

  FILE *outfp;

  outfp = fopen(partName, "w");

  // Write out the file FS descriptor
  fwrite(&filesys, sizeof(filesys), 1, outfp);

  // Write out the directory
  fseek(outfp, filesys.dirByteIndex, SEEK_SET);
  fwrite(directory, sizeof(TDirectory), filesys.maxFiles, outfp);

  // Seek to start of bitmap table
  fseek(outfp, filesys.bitmapByteIndex, SEEK_SET);

  // Write the bitmap
  fwrite(bitmap, filesys.bitmapLen, 1, outfp);

  // Write the inode table
  fseek(outfp, filesys.inodeByteIndex, SEEK_SET);
  for(i=0; i<filesys.maxFiles; i++)
    fwrite(inodeTable[i], sizeof(unsigned long), filesys.numInodeEntries, outfp);

  // Write out the data
  fseek(outfp, filesys.fsSize, SEEK_SET);
  fprintf(outfp, "!");
  fclose(outfp);

  printf("\nFILE SYSTEM SUMMARY\n");
  printf(  "===================\n\n");
  printf("File system size: %lu bytes\n", filesys.fsSize);
  printf("Block size: %u bytes\n", filesys.blockSize);
  printf("Maximum number of files: %u\n", filesys.maxFiles);
  printf("Number of blocks: %u\n", filesys.numBlocks);
  printf("Bitmap Length: %u bytes\n", filesys.bitmapLen);
  printf("Usable Data Space: %lu bytes\n", usableSpace);
  printf("Number of pointers per inode block: %u\n", filesys.numInodeEntries);
  printf("Percentage Usable Data Space: %3.2g%%\n", (double) usableSpace / filesys.fsSize * 100.0);

  printf("\nByte Indexes:\n\n");

  printf("Directory Index: %u\n", filesys.dirByteIndex);
  printf("Bitmap Index: %u\n", filesys.bitmapByteIndex);
  printf("Inode Index: %u\n", filesys.inodeByteIndex);
  printf("Data Index: %u\n\n", filesys.dataByteIndex);

  free(directory);
  free(bitmap);

  for(i=0; i<filesys.maxFiles; i++)
    free(inodeTable[i]);

  free(inodeTable);
}

