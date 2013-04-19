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

int
main (int argc, char *argv[])
{
  if (argc < 4)
    {
      printf ("recover UID GID [filename]\n");
      exit (-1);
    }

  search_uid = atoi (argv[1]);

  if (search_uid < 0)
    {
      perror ("UID is < 0 and invalid.");
      exit (-1);
    }

  search_gid = atoi (argv[2]);

  if (argv[2] < 0)
    {
      perror ("GID is < 0 and invalid.");
      exit (-1);
    }

  // Open as read only to prevent damage to the datafile
  data_file_descriptor = open (argv[3], O_RDONLY);
  if (data_file_descriptor == -1)
    {
      perror ("Couldn't open file.");
      exit (-1);
    }

  struct stat data_file_stat;
  fstat (data_file_descriptor, &data_file_stat);

  printf ("Data file size in bytes: %lu\n", data_file_stat.st_size);


  data_file_map_start = mmap (NULL, data_file_stat.st_size, PROT_READ,
			      MAP_PRIVATE, data_file_descriptor, 0);

  if (data_file_map_start == MAP_FAILED)
    {
      close (data_file_descriptor);
      perror ("Memory map failed.");
      exit (-1);
    }

  data_file_map_end = data_file_map_start + data_file_stat.st_size;

  max_block_number = data_file_stat.st_size / BLOCK_SIZE;

  void *search_ptr = data_file_map_start;

  // Search for a new inode that forfills one of our properties

  inode_t *f = find_inode (search_ptr, data_file_map_end,
			   search_uid, search_gid);
  int file_num = 0;
  while (f != NULL)
    {
      // Print found inode
      print_inode (f);

      // Check that the inode is valid

      if (check_inode (f) == GEN_ERROR)
	{
	  perror ("INODE INVALID!");
	}
      else
	{
          // This buffer should be large enough to hold any filename
          char output_file_name[15];
          sprintf(output_file_name, "%d", file_num);
	  FILE *output_file_ptr = fopen (output_file_name, "w");
	  if (output_file_ptr == NULL)
	    {
	      perror ("Could not open output file.");
	      exit (-1);
	    }

	  // Copy direct blocks from mapped datafile to output file in order
          printf("Copying DBLOCKS.\n");
	  int data_copied = 0;
	  int dblock_pos;
	  for (dblock_pos = 0; dblock_pos < N_DBLOCKS; dblock_pos++)
	    {
	      if (f->dblocks[dblock_pos] > 0)
		{
		  int bytes_copied = copy_dblock (data_file_map_start,
                                                  output_file_ptr,
                                                  f->dblocks[dblock_pos],
                                                  f->size - data_copied);
		  if ((bytes_copied == GEN_ERROR) ||
                      (bytes_copied == BLOCK_COPY_ERROR))
		    {
		      perror ("Error copying direct blocks to output file.");
		      exit (-1);
		    }
		  else if (bytes_copied >= 0)
		    {
		      data_copied += bytes_copied;
		    }
		}
	      else
		{
		  printf ("No more data in direct blocks!\n");
		  break;
		}
	    }

	  printf ("%2.2f %% copied.\n",
		  ((float) data_copied / f->size) * 100);
	  printf ("%i/%i\n", data_copied, f->size);

	  // For each I1 block access it and gather the block #s it contains
	  if (data_copied < f->size)
	    {
              printf("Copying I1BLOCKS.\n");
	      int iblock_pos;
	      for (iblock_pos = 0; iblock_pos < N_IBLOCKS; iblock_pos++)
		{
		  if (f->iblocks[iblock_pos] > 0)
		    {
		      int bytes_copied = copy_i1block (data_file_map_start,
						       output_file_ptr,
						       f->iblocks[iblock_pos],
						       f->size - data_copied);
		      if (bytes_copied == BLOCK_COPY_ERROR)
			{
			  perror("Error copying indirect blocks to output file.");
			  exit (-1);
			}
		      else if (bytes_copied >= 0)
			{
			  data_copied += bytes_copied;
			}
		    }
		  else
		    {
		      printf ("No more valid indirect blocks!\n");
		      break;
		    }
		}
	    }

	  printf ("%2.2f %% copied.\n",
		  ((float) data_copied / f->size) * 100);
	  printf ("%i/%i\n", data_copied, f->size);

	  // For the I2 block access it, and build an array of the I1 blocks it
	  // points to.
	  // Then iterate through this array and use copy_i1block
	  if (data_copied < f->size)
	    {
              printf("Copying I2BLOCK.\n");
	      int i1block_buffer[BLOCK_SIZE / sizeof (int)];
	      int i1block_count =
		unwind_iblock (data_file_map_start, f->i2block,
			       i1block_buffer);
	      if (i1block_count == NO_POINTERS_IN_IBLOCK)
		{
                  // I don't know if this should be fatal or not
		  perror ("No pointers found in I2BLOCK!");
		  exit (-1);
		}
	      int i1block_pos;
	      for (i1block_pos = 0; i1block_pos < i1block_count;
		   i1block_pos++)
		{
		  if (i1block_buffer[i1block_pos] > 0)
		    {
                      printf("Attempting to access IBLOCK: %i\n",
                             i1block_buffer[i1block_pos]);
                      if (i1block_buffer[i1block_pos] > data_file_stat.st_size
                          / BLOCK_SIZE)
                        {
                          // If we have a a block that is wildly out of range
                          printf("IBLOCK out of range!\n");
                          continue;
                        }
		      int bytes_copied = copy_i1block (data_file_map_start,
						       output_file_ptr,
						       i1block_buffer
						       [i1block_pos],
						       f->size - data_copied);
		      if (bytes_copied == BLOCK_COPY_ERROR)
			{
			  perror ("Error copying I2 blocks to output file.");
			  exit (-1);
			}
		      else if (bytes_copied >= 0)
			{
			  data_copied += bytes_copied;
			}
		    }
		  else
		    {
		      printf ("No more valid I2 blocks!\n");
		      break;
		    }
		}
	    }

	  printf ("%2.2f %% copied.\n",
		  ((float) data_copied / f->size) * 100);
	  printf ("%i/%i\n", data_copied, f->size);

	 /* For the I3 block access it, and build an array of I2 blocks it 
	points to. Iterate through this array and for each one, access it and
	build an array of the I1 blocks it points to. Then iterate through this
	array and use copy_i1block. */
	if (data_copied < f->size)
	    {
              printf("Copying I3BLOCK.\n");
        int i2block_buffer[BLOCK_SIZE / sizeof (int)];
        int i2block_count = unwind_iblock (data_file_map_start, f->i3block, i2block_buffer);
    	// if there are no pointers found in our I3 block, exit
       	if (i2block_count == NO_POINTERS_IN_IBLOCK)
        	{
        		perror ("No pointers found in I3BLOCK!");
        		exit (-1);
        	}
	    	int i2block_pos;
	    	for (i2block_pos = 0; i2block_pos < i2block_count; i2block_pos++)
	    	{
	    		if (i2block_buffer[i2block_pos] >= 0)
	    		{
	    			printf("Attempting to access I2BLOCK: %i\n", i2block_buffer[i2block_pos]);
	    			if(i2block_buffer[i2block_pos] > data_file_stat.st.size / BLOCK_SIZE)
	    			{
	    				// If we have a block that is wildly out of range
	    				printf("I2BLOCK out of range!\n");
	    				continue;
	    			}
	    		i1block_count = unwind_iblock (data_file_map_start, f->i2block, i1block_buffer);
	    		if (i1block_count == NO_POINTERS_IN_IBLOCK)
	    		{
	    			perror ("No pointers found in I2BLOCK!");
	    			exit(-1);
	    		}
	    		for (i1block_pos = 0, i1block_pos < i1block_count; i1block_pos++)
	    		{
	    			if(i1block_buffer[i1block_pos] > 0)
	    			{
	    				printf("Attempting to access IBLOCK: %i\n", i1block_buffer[i1block_pos]);
	    				if (i1block_buffer[i1block_pos] > data_file_stat.st_size / BLOCK_SIZE)
{
	printf("IBLOCK out of range!\n");
	continue;
}
int bytes_copied = copy_i1block (data_file_map_start,
	output_file_ptr,
	i1block_buffer
	[i1block_pos],
	f->size - data_copied);
	if (bytes_copied == BLOCK_COPY_ERROR)
	{
		perror("Error copying I3 blocks to output file");
		exit(-1);
	}
	else if (bytes_copied >=0)
	{
		data_copied += bytes_copied;
	}
	    			}
	    			else
	    			{
	    				printf ("No more valid I3 blocks!\n");
	    				break;
	    			}
	    		}
	    		}
	    	}
	    	}

	  printf ("%2.2f %% copied.\n",
		  ((float) data_copied / f->size) * 100);
	  printf ("%i/%i\n", data_copied, f->size);
	}
      search_ptr = f + 1;
      f = find_inode(search_ptr, data_file_map_end,
                     search_uid, search_gid);
      file_num ++;
    }

  // Unmap the data file from memory
  if (munmap (data_file_map_start, data_file_stat.st_size) == -1)
    {
      perror ("Error unmapping data file.");
      close (data_file_descriptor);
      exit (-1);
    }
  return 0;
};

inode_t *
find_inode (void *start_ptr, void *end_ptr, int uid, int gid)
{
  printf("Searching for INODE.\n");
  printf("Starting at: %p\n", start_ptr);
  int *search_ptr = (int*) start_ptr;
  while (search_ptr <= end_ptr - sizeof(int))
    {
      if ((search_ptr[0] == uid) && (search_ptr[1] == gid))
	{
          return search_ptr - 2;
	}
      search_ptr++;
    }
  return NULL;
}

int
check_inode (inode_t * node)
{
  // Test inode properties against known requirements to prevent bogus records.
  // This means checking that all block numbers are <= DF.size/BL.size

  int dblock_pos;
  for (dblock_pos = 0; dblock_pos < N_DBLOCKS; dblock_pos++)
    {
      if ((node->dblocks[dblock_pos] < 0) ||
	  (node->dblocks[dblock_pos] >= max_block_number))
	{
	  perror ("DBLOCK OUT OF RANGE!");
	  return GEN_ERROR;
	}
    }

  int iblock_pos;
  for (iblock_pos = 0; iblock_pos < N_IBLOCKS; iblock_pos++)
    {
      if ((node->iblocks[iblock_pos] < 0) ||
	  (node->iblocks[iblock_pos] >= max_block_number))
	{
	  perror ("IBLOCK OUT OF RANGE!");
	  return GEN_ERROR;
	}
    }

  if ((node->i2block < 0) || (node->i2block >= max_block_number))
    {
      perror ("I2BLOCK OUT OF RANGE!");
      return GEN_ERROR;
    }

  if ((node->i3block < 0) || (node->i3block >= max_block_number))
    {
      perror ("I3BLOCK OUT OF RANGE!");
      return GEN_ERROR;
    }
  return 0;
}

void
print_inode (inode_t * node)
{
  if (node == NULL)
    {
      printf ("NULL POINTER\n");
      return;
    }
  printf ("address: %p\n", node);
  printf ("foo: %i\n", node->foo);
  printf ("nlink: %i\n", node->nlink);
  printf ("uid: %i\n", node->uid);
  printf ("gid: %i\n", node->gid);
  printf ("size: %i\n", node->size);
  printf ("ctime: %i\n", node->ctime);
  printf ("mtime: %i\n", node->mtime);
  printf ("atime: %i\n", node->atime);
  int i;
  for (i = 0; i < N_DBLOCKS; i++)
    {
      printf ("dblock (%i): %i\n", i, node->dblocks[i]);
    }
  for (i = 0; i < N_IBLOCKS; i++)
    {
      printf ("iblock (%i): %i\n", i, node->iblocks[i]);
    }
  printf ("i2block: %i\n", node->i2block);
  printf ("i3block: %i\n", node->i3block);
};

int
copy_dblock (void *from_ptr, FILE * to_ptr, int block_num, int bytes)
{
  if (bytes < 0)
    {
      perror ("Bytes to copy requested < 0.");
      return NEG_BYTE_REQ;
    }

  if (bytes > BLOCK_SIZE)
    {
      bytes = BLOCK_SIZE;
    }
  void *copy_buffer = malloc (bytes);
  if (copy_buffer == NULL)
    {
      perror ("Could not malloc copy buffer.");
      return GEN_ERROR;
    }
  memcpy (copy_buffer, from_ptr + (BLOCK_SIZE * block_num), bytes);
  if (fwrite (copy_buffer, 1, bytes, to_ptr) != bytes)
    {
      perror ("Unsuccessful block data copy!");
      return BLOCK_COPY_ERROR;
    }
  free (copy_buffer);
  return bytes;
};

int
copy_i1block (void *from_ptr, FILE * to_ptr, int block_num, int bytes)
{
  if (bytes < 0)
    {
      perror ("Bytes to copy requested < 0.");
      return NEG_BYTE_REQ;
    }
  int block_buffer[BLOCK_SIZE / sizeof (int)];
  int block_count = unwind_iblock (from_ptr, block_num, block_buffer);
  if (block_count == NO_POINTERS_IN_IBLOCK)
    {
      perror ("No pointers in I1BLOCK!");
      return NO_POINTERS_IN_IBLOCK;
    }
  int block_pos;
  int data_copied = 0;
  for (block_pos = 0; block_pos < block_count; block_pos++)
    {
      int bytes_copied = copy_dblock (from_ptr,
				     to_ptr,
				     block_buffer[block_pos],
				     bytes - data_copied);
      if ( (bytes_copied == BLOCK_COPY_ERROR) ||
           (bytes_copied == GEN_ERROR))
	{
	  perror ("Error copying direct blocks to output file.");
	  return BLOCK_COPY_ERROR;
	}
      else
	{
	  data_copied += bytes_copied;
	}
    }
  return data_copied;
};

int
unwind_iblock (void *source_ptr, int block_num, int *block_buffer)
{
  memcpy (block_buffer, source_ptr + (BLOCK_SIZE * block_num), BLOCK_SIZE);
  int block_pos;
  for (block_pos = 0; block_pos < BLOCK_SIZE / sizeof (int); block_pos++)
    {
      if (block_buffer[block_pos] < 0)
	{
          printf("Block (%i): %i\n", block_pos, block_buffer[block_pos]);
	  break;
	}
    }
  if (block_buffer[block_pos] == 0)
    {
      // This is the case that there were no indirect blocks pointed to.
      return NO_POINTERS_IN_IBLOCK;
    }
  printf("Returning %i block pointers.\n", block_pos);
  return block_pos;
};
