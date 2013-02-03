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

      // We want to know how many tokens are being used in the buffer
      unsigned int token_buf_len = 0;
      // If we hit a NULL or reach the last addressable element of the array we should stop
      while((token_buf[token_buf_len] != NULL) && (token_buf_len < token_buf_size)) token_buf_len++;
      if( token_buf_len > 0 )
	{
	  // Sloppy exit function
	  if( strcmp(token_buf[0], "exit") == 0) exit_status = true;
	  else
	    {
	      // Now it's time to fork!
	      pid_t child_pid = fork();

	      if(child_pid >= 0)
		{
		  // Our fork was succesful!
		  if(child_pid == 0)
		    {
		      // We're in the child!
		      execvp(token_buf[0], token_buf);
		    }
		  else
		    {
		      // We're in the parent!
		      //printf("In the parent!\n");
		      // If last token is not backgrounding wait for child to terminate
		      int status;
		      waitpid(child_pid, &status, 0);
		    }
		}
	      else
		{
		  // Fork failed!
		  perror("Fork was unsuccesful in creating a new process.");
		  exit(-1);
		}
	    }
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
