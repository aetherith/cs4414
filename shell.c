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

      // Store where in the token buffer we want to start processing tokens at
      unsigned int start_token_proc_at = 0;

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
		      unsigned int redirect_token_pos = find_redirect(token_buf, &token_buf_len, &start_token_proc_at);
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
		      // Problem in here regarding how many parameters there are (when redirecting we chop off one)
		      unsigned int param_buf_size;
		      if( redirect_token_pos > 0 ) param_buf_size = redirect_token_pos;
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

		      for( i = 0; i < param_buf_size; i++ ) printf("%s\n", param_buf[i]);

		      // Now we set up redirection/dumping output to /dev/null for background processes
		      // Dumping background output if the command doesn't already redirect to a file
		      if( ( redirect_token_pos == 0) && ( is_background == true ) )
			{
			  fclose(stdout);
			  stdout = fopen("/dev/null", "w");
			  if( stdout == NULL )
			    {
			      perror("Couldn't open /dev/null for writing.");
			      exit(-1);
			    }
			}
		      
		      // Now we deal with all other redirection
		      if( redirect_token_pos > 0 )
			{
			  if( strcmp(token_buf[redirect_token_pos], ">") == 0 )
			    {
			      fclose(stdout);
			      stdout = fopen(redirect_file_name, "w");
			      if( stdout == NULL )
				{
				  perror("Couldn't open output file for writing.");
				  exit(-1);
				}
			    }
			  if( strcmp(token_buf[redirect_token_pos], ">>") == 0 )
			    {
			      fclose(stdout);
			      stdout = fopen(redirect_file_name, "a");
			      if( stdout == NULL )
				{
				  perror("Couldn't open output file for appending.");
				  exit(-1);
				}
			    }
			  if( strcmp(token_buf[redirect_token_pos], "<") == 0 )
			    {
			      fclose(stdin);
			      stdin = fopen(redirect_file_name, "r");
			      if( stdin == NULL )
				{
				  perror("Couldn't open input file for reading.");
				  exit(-1);
				}
			    }
			}

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

unsigned int find_redirect(char **token_buf, unsigned int *token_buf_len, unsigned int *start_pos)
{
  unsigned int i;
  for(i = *start_pos; i < *token_buf_len; i++)
    {
      if( (strcmp(token_buf[i], ">") == 0) ||
	  (strcmp(token_buf[i], ">>") == 0) ||
	  (strcmp(token_buf[i], "<") == 0)) return i;
    }
  return 0;
};

unsigned int find_pipe(char **token_buf, unsigned int *token_buf_len, unsigned int *start_pos){
  unsigned int i;
  for( i=*start_pos; i < *token_buf_len; i++ )
    {
      if( strcmp(token_buf[i], "|") == 0) return i;
    }
  return 0;
};
