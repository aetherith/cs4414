/* Thomas Foulds (tcf9bj)
 * Amanda Ray (ajr2fu)
 * Sami Fekadu (sf5fw)
 * CS4414
 * 04/14/13
 * Assignment 6
 * recover.c
 */

#include "recover.h"

static int search_gid;
static int search_uid;
static int data_file_descriptor = -1;
static void *data_file_map_start;
static void *data_file_map_end;
static int max_block_number;

int main ( int argc, char* argv[] )
{
  if ( argc < 4 )
    {
      printf( "recover UID GID [filename]\n" );
      exit( -1 );
    }

  search_uid = atoi( argv[1] );

  if ( search_uid < 0 )
    {
      perror( "UID is < 0 and invalid." );
      exit( -1 );
    }
  
  search_gid = atoi( argv[2] );

  if ( argv[2] < 0 )
    {
      perror( "GID is < 0 and invalid." );
      exit( -1 );
    }

  // Open as read only to prevent damage to the datafile
  data_file_descriptor = open( argv[3], O_RDONLY );
  if ( data_file_descriptor == -1 )
    {
      perror( "Couldn't open file." );
      exit( -1 );
    }

  struct stat data_file_stat;
  fstat( data_file_descriptor, &data_file_stat );

  printf("Data file size in bytes: %lu\n", data_file_stat.st_size);
  

  data_file_map_start = mmap( NULL, data_file_stat.st_size, PROT_READ,
                              MAP_PRIVATE, data_file_descriptor, 0 );

  if ( data_file_map_start == MAP_FAILED )
    {
      close( data_file_descriptor );
      perror( "Memory map failed." );
      exit( -1 );
    }
  
  data_file_map_end = data_file_map_start + data_file_stat.st_size;

  max_block_number = data_file_stat.st_size / BLOCK_SIZE;

  void *search_ptr = data_file_map_start;
  
  // Search for a new inode that forfills one of our properties

  inode_t *f = find_inode( search_ptr, data_file_map_end,
                           search_uid, search_gid );
  // Print found inode
  print_inode( f );

  // Check that the inode is valid

  check_inode( f );

  // Create output file of size specified by inode and memory map it

  // Memcopy direct blocks from mapped datafile to output file in order

  // For each indirect block access it and gather the block #s it contains

  //

  // Unmap the file from memory
  if ( munmap( data_file_map_start, data_file_stat.st_size ) == -1 )
    {
      perror( "Error unmapping data file." );
      close( data_file_descriptor );
      exit( -1 );
    }
  return 0;
};

inode_t* find_inode( void *start_ptr, void *end_ptr, int uid, int gid )
{
  printf("== find_inode ==\n");
  printf("Searching for:\nUID: %i\nGID: %i\n", uid, gid);
  int *search_ptr = (int*) start_ptr;
  while ( search_ptr != end_ptr )
    {
      if ( *search_ptr == gid )
        {
          printf("GID match found.\n");
          return search_ptr - 3;
        }
      if ( *search_ptr == uid )
        {
          printf("UID match found.\n");
          // Admittedly the -2 is a magic number to make it line up properly
          // in the inode_t structure.
          return search_ptr - 2;
        }
      search_ptr++;
    }
  printf("!= find_inode - NULL =!\n");
  return NULL;
}

int check_inode ( inode_t *node )
{
  // Test inode properties against known requirements to prevent bogus records.
  // This means checking that all block numbers are <= DF.size/BL.size

  int dblock_pos = 0;
  for ( dblock_pos; dblock_pos < N_DBLOCKS; dblock_pos++ )
    {
      if ( ( node->dblocks[dblock_pos] < 0 ) || 
           ( node->dblocks[dblock_pos] >= max_block_number ) )
        {
          printf("DBLOCK OUT OF RANGE!\n");
          return -1;
        }
    }

  int iblock_pos = 0;
  for ( iblock_pos; iblock_pos < N_IBLOCKS; iblock_pos++ )
    {
      if ( ( node->iblocks[iblock_pos] < 0 ) ||
           ( node->iblocks[iblock_pos] >= max_block_number ) )
        {
          printf("IBLOCK OUT OF RANGE!\n");
          return -1;
        }
    }

  if ( ( node->i2block < 0 ) || ( node->i2block >= max_block_number ) )
    {
      printf("I2BLOCK OUT OF RANGE!\n");
      return -1;
    }

  if ( ( node->i3block < 0 ) || ( node->i3block >= max_block_number ) )
    {
      printf("I3BLOCK OUT OF RANGE!\n");
      return -1;
    }
  return 0;
}

void print_inode ( inode_t *node )
{
  if ( node == NULL )
    {
      printf( "NULL POINTER\n" );
      return;
    }
  printf( "foo: %i\n", node->foo );
  printf( "nlink: %i\n", node->nlink );
  printf( "uid: %i\n", node->uid );
  printf( "gid: %i\n", node->gid );
  printf( "size: %i\n", node->size );
  printf( "ctime: %i\n", node->ctime );
  printf( "mtime: %i\n", node->mtime );
  printf( "atime: %i\n", node->atime );
  int i;
  for ( i = 0; i < N_DBLOCKS; i++ )
    {
      printf( "dblock (%i): %i\n", i, node->dblocks[i] );
    }
  for ( i = 0; i < N_IBLOCKS; i++ )
    {
      printf( "iblock (%i): %i\n", i, node->iblocks[i] );
    }
  printf( "i2block: %i\n", node->i2block );
  printf( "i3block: %i\n", node->i3block );
};
