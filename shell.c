/* Thomas Foulds
 * tcf9bj
 * 02/01/13
 * CS4414
 * shell.c
 */

#include "shell.h"

int main(int argc, char* argv[])
{
  unsigned int buf_base_size = 80;
  unsigned int buf_size = buf_base_size;
  // Allocate an input buffer, we'll resize it if need be
  char *input_buf = malloc(buf_size*sizeof(char));

  printf("Welcome to the shell.\n");
  bool exit_status = false;
  while(exit_status == false)
    {
      printf("$ ");
      char in_char = '\0';
      unsigned int buf_pos = 0;
      while( in_char != '\n' )
	{
	  // If we've hit the end of our buffer without getting a newline then we should resize the buffer up
	  if( buf_pos == buf_size )
	    {
	      buf_size += buf_size/2;
	      input_buf = realloc(input_buf, buf_size*sizeof(char));
	      if(input_buf == NULL)
		{
		  perror("Error increasing size of input buffer.");
		  exit(-1);
		}
	    }
	  
	  // Read characters and put them in the input buffer
	  in_char = getchar();
	  input_buf[buf_pos] = in_char;
	  buf_pos++;
	}
      // Here is where we process the input string
      
      

      // Sloppy exit function, needs improving.  Just here so I can get out for the time being.
      if( strcmp(input_buf, "exit\n") == 0)
	{
	  printf("Exiting the shell...\n");
	  exit_status = true;
	}

      // Now we reallocate our buffer down to the original size and fill it with \0
      input_buf = realloc(input_buf, buf_base_size*sizeof(char));
      if(input_buf == NULL)
	{
	  perror("Error decreasing size of input buffer.");
	  exit(-1);
	}
      memset(input_buf,'\0',buf_base_size*sizeof(char));
      
    }

  free(input_buf);
  return 0;
};
