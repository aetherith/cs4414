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
	      buf_size += buf_base_size/2;
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
      
      // We're going to want to tokenize our input, so here's an array of char strings
      unsigned int token_buf_base_size = 20;
      unsigned int token_buf_size = token_buf_base_size;
      char** token_buf = malloc(token_buf_base_size*sizeof(char*));
      // Now we want to fill it with NULL pointers because it's easy to identify those!
      unsigned int token_pos;
      for(token_pos = 0; token_pos < token_buf_size; token_pos++)
	{
	  token_buf[token_pos] = NULL;
	}
      
      // We're going to look through our input buffer and make up tokens based on where spaces occur
      char *token = strtok(input_buf, " \n");
      token_pos = 0;
      while(token != NULL)
	{
	  token_buf[token_pos] = token;
	  token = strtok(NULL, " \n");
	  token_pos++;
	  if(token_pos == token_buf_size)
	    {
	      token_buf_size += token_buf_base_size/2;
	      token_buf = realloc(token_buf, token_buf_size*sizeof(char*));
	      if(token_buf == NULL)
		{
		  perror("Error increasing size of token buffer.");
		  exit(-1);
		}
	    }
	}

      for(token_pos = 0; token_pos < token_buf_size; token_pos++)
	{
	  if(token_buf[token_pos] != NULL) printf("%s\n", token_buf[token_pos]);
	  else break;
	}



      // Sloppy exit function, needs improving.  Just here so I can get out for the time being.
      // Only accepts the exit command if it is entered on a line by itself
      if( strcmp(token_buf[0], "exit") == 0)
	{
	  printf("Exiting the shell...\n");
	  exit_status = true;
	}

      free(token_buf);

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
