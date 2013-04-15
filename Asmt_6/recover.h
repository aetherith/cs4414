/* Thomas Foulds (tcf9bj)
 * Amanda Ray (arj2fu)
 * Sami 
 * CS4414
 * 04/14/13
 * Assignment 6
 * recover.h
 */

#ifndef _RECOVER_H
#define _RECOVER_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>

#define N_DBLOCKS 12
#define N_IBLOCKS 3

#define GIF87A_SIGNATURE 0x474946383761           // .gif
#define GIF89A_SIGNATURE 0x474946383961           // .gif
#define TIF_SIGNATURE    0x49492A00               // .tif
#define TIFF_SIGNATURE   0x4D4D002A               // .tiff
#define EXE_SIGNATURE    0x4D5A                   // .exe
#define PNG_SIGNATURE    0x89504E470D0A1A0A       // .png
#define UTF8_SIGNATURE   0xEFBBBF                 // ASCII, .html
#define PDF_SIGNATURE    0x25504446               // .pdf
#define PS_SIGNATURE     0x2521                   // .ps
// Need .jpeg/.jpg

struct inode { 
  int foo;      /* Unknown field */
  int nlink;    /* Number of links to this file */
  int uid;      /* Owner's user ID */
  int gid;      /* Owner's group ID */
  int size;     /* Number of bytes in file */
  int ctime;    /* Time field */
  int mtime;    /* Time field */
  int atime;    /* Time field */
  int dblocks[N_DBLOCKS]; /* Pointers to data blocks */ 
  int iblocks[N_IBLOCKS]; /* Pointers to indirect blocks */ 
  int i2block; /* Pointer to doubly indirect block */
  int i3block; /* Pointer to triply indirect block */
};



#endif