/* Thomas Foulds (tcf9bj)
 * Amanda Ray (ajr2fu)
 * Sami Fekadu (sf5fw)
 * CS4414
 * 04/14/13
 * Assignment 6
 * recover.h
 */

#ifndef _RECOVER_H
#define _RECOVER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define N_DBLOCKS 12
#define N_IBLOCKS 3

#define BLOCK_SIZE 1024

#define GIF87A_SIGNATURE 0x474946383761	// .gif
#define GIF89A_SIGNATURE 0x474946383961	// .gif
#define TIF_SIGNATURE    0x49492A00	// .tif
#define TIFF_SIGNATURE   0x4D4D002A	// .tiff
#define EXE_SIGNATURE    0x4D5A	// .exe
#define PNG_SIGNATURE    0x89504E470D0A1A0A	// .png
#define UTF8_SIGNATURE   0xEFBBBF	// ASCII, .html
#define PDF_SIGNATURE    0x25504446	// .pdf
#define PS_SIGNATURE     0x2521	// .ps
#define JPG_SIGNATURE    0xFFD8FFE0	// .jpg
#define JPEG_SIGNATURE   0x0	// .jpeg

#define GEN_ERROR -1
#define NEG_BYTE_REQ -2
#define NO_POINTERS_IN_IBLOCK -3
#define BLOCK_COPY_ERROR -4

struct inode
{
  int foo;			/* Unknown field */
  int nlink;			/* Number of links to this file */
  int uid;			/* Owner's user ID */
  int gid;			/* Owner's group ID */
  int size;			/* Number of bytes in file */
  int ctime;			/* Time field */
  int mtime;			/* Time field */
  int atime;			/* Time field */
  int dblocks[N_DBLOCKS];	/* Pointers to data blocks */
  int iblocks[N_IBLOCKS];	/* Pointers to indirect blocks */
  int i2block;			/* Pointer to doubly indirect block */
  int i3block;			/* Pointer to triply indirect block */
};

typedef struct inode inode_t;

// Finds an inode structure with either UID or GID that matches those passed
// in the call.
// void *start_ptr is where in the memory mapped source file the search should
// begin from.
// void *end_ptr is the last valid address in the memory mapped source file.
// Returns pointer to found inode on success, NULL pointer on failure.
inode_t *find_inode (void *start_ptr, void *end_ptr, int uid, int gid);

// Checks inode contents against known requirements to prevent bogus records
// by making sure all block numbers referenced are valid.
// Returns 0 if inode is valid, GEN_ERROR otherwise.
int check_inode (inode_t * node);

// Prints content of inode in a formatted fashion.
void print_inode (inode_t * node);

// Copies block data from a memory mapped file to a FILE pointer.
// void *from_ptr should be a pointer to the beginning of the source mapping.
// void *to_ptr should be a FILE pointer to the output file insertion point.
// Returns number of bytes copied on successful copy.
// Returns GEN_ERROR, NEG_BYTE_REQ, or BLOCK_COPY_ERROR on error.
// Not all errors are fatal and caller should manage accordingly.
int copy_dblock (void *from_ptr, FILE * to_ptr, int block_num, int bytes);

// Unwinds an I1 block and copies its DBLOCKS to FILE pointer.
// Returns number of bytes copied on successful copy.
// Returns NEG_BYTE_REQ, NO_POINTERS_IN_IBLOCK, or BLOCK_COPY_ERROR on error.
int copy_i1block (void *from_ptr, FILE * to_ptr, int block_num, int bytes);

// Follows the indirection of an I(n) block and copies the found block numbers
// into the integer array *block_buffer.
// Assumes that any empty pointers in the IBLOCK are zeroed, not garbage filled.
// int *block_buffer is expected to be BLOCK_SIZE/sizeof( int )
// Returns number of valid blocks found if successful.
// Returns NO_POINTERS_IN_IBLOCK on failure.
int unwind_iblock (void *source_ptr, int block_num, int *block_buffer);

#endif
