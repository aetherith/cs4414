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
static void *data_file_map;

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

  data_file_map = mmap( NULL, data_file_stat.st_size, PROT_READ, MAP_PRIVATE,
                        data_file_descriptor, 0 );

  if ( data_file_map == MAP_FAILED )
    {
      close( data_file_descriptor );
      perror( "Memory map failed." );
      exit( -1 );
    }

  print_inode( find_inode( data_file_map, data_file_map + data_file_stat.st_size
                           , search_uid, search_gid) );

  // Unmap the file from memory
  if ( munmap( data_file_map, data_file_stat.st_size ) == -1 )
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
  inode_t *search_ptr = (inode_t*) start_ptr;
  while ( search_ptr + sizeof( inode_t ) <= end_ptr )
    {
      if ( search_ptr->uid == uid || search_ptr->gid == gid )
        {
          printf("!= find_inode - !NULL =!\n");
          return search_ptr;
        }
      search_ptr += sizeof( inode_t );
    }
  printf("!= find_inode - NULL =!\n");
  return NULL;
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
