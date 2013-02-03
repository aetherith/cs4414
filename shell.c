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

      /*
      // redirect_mode takes on one of 4 values 0 - no redirect, 1 - redirect overwrite, 2 - redirect append, 3 - redirect input
      int redirect_mode = 0;
      char *redirect_file_name;

      // If we found a redirection operator in a valid position with another token following it, set the next token as the file to redirect to
      // Else set the file to redirect to as ""
      if(redirect_token_pos > 0)
	{
	  // If we have space in the buffer to have an additional filename
	  if( redirect_token_pos + 1 < token_buf_len ) redirect_file_name = token_buf[redirect_token_pos + 1];
	  else redirect_file_name = "";
	  // Now we set the type of redirection
	  if( strcmp(token_buf[redirect_token_pos], ">") == 0 ) redirect_mode = 1;
	  else if( strcmp(token_buf[redirect_token_pos], ">>") == 0) redirect_mode = 2;
	  else if( strcmp(token_buf[redirect_token_pos], "<") == 0) redirect_mode = 3;
	}
      else redirect_file_name = "";
      printf("Redirect mode: %i\n", redirect_mode);

      // I think the trick is to create the new file descriptors here and then just have the child do the same thing every time
      FILE *input_file = stdin;
      FILE *output_file = stdout;

      // Now we want to create a new array of parameters for the child
      unsigned int param_buf_size;
      if(redirect_token_pos > 1) param_buf_size = redirect_token_pos - 1;
      else if( redirect_token_pos == 1) param_buf_size = 1;
      else param_buf_size = token_buf_len;

      // We need space for all the parameters as well as the terminating NULL pointer
      char *param_buf[param_buf_size + 1];

      unsigned int i;
      for( i = 0; i < param_buf_size; i++)
	{
	  // We are assuming that the background operator will only ever appear at the end of a line
	  if( strcmp(token_buf[i], "&") == 0 ) break;
	  param_buf[i] = token_buf[i];
	}
      param_buf[param_buf_size] = NULL;
      */

      // If we have valid tokens in the buffer
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
		      // Look for the first file redirection opereator and store its position
		      unsigned int redirect_token_pos = find_redirect(token_buf, &token_buf_len);
		      char* redirect_file_name;
		      
		      // If we find a redirection operator in a valid position with another token folowing it, set the next token as the file to redirect to
		      // Else set the file to redirect to as ""
		      if( redirect_token_pos > 0 )
			{
			  // If we have space in the buffer to have an additonal filename
			  if( redirect_token_pos + 1 < token_buf_len ) redirect_file_name = token_buf[redirect_token_pos + 1];
			  else redirect_file_name = "";
			}

		      // Now we want to create a new array of parameters for the child
		      unsigned int param_buf_size;
		      if( redirect_token_pos > 1 ) param_buf_size = redirect_token_pos - 1;
		      else if( redirect_token_pos == 1 ) param_buf_size = 1;
		      else param_buf_size = token_buf_len;

		      // We need space for all the parameters as well as the terminating NULL pointer
		      char *param_buf[param_buf_size + 1];

		      bool is_background = false;
		      unsigned int i;
		      for( i = 0; i < param_buf_size; i++)
			{
			  // We are assuming that the background operator will only ever appear at the end of a line
			  if( strcmp(token_buf[i], "&") == 0 )
			    {
			      is_background = true;
			      break;
			    }
			  param_buf[i] = token_buf[i];
			}
		      param_buf[param_buf_size] = NULL;

		      // Now we set up redirection/dumping output to /dev/null for background processes

		      int retval = execvp(param_buf[0], param_buf);
		      if( retval == -1 )
			{
			  printf("Could not exec: %s\n", param_buf[0]);
			  exit(-1);
			}
		    }
		  else
		    {
		      // We're in the parent!
		      // If last token is not backgrounding wait for child to terminate
		      if( strcmp(token_buf[token_buf_len - 1], "&") != 0 )
			{
			  int status;
			  waitpid(child_pid, &status, 0);
			}
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

unsigned int find_redirect(char **token_buf, unsigned int *token_buf_len)
{
  unsigned int i;
  for(i = 0; i < *token_buf_len; i++)
    {
      if( (strcmp(token_buf[i], ">") == 0) ||
	  (strcmp(token_buf[i], ">>") == 0) ||
	  (strcmp(token_buf[i], "<") == 0)) return i;
    }
  return 0;
};
