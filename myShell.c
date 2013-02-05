#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

// ******** Function Prototypes ********

int check_Pipe(char * cmdbuf[]);   // check if the command contains a pipe symbol "|"

int check_Redirection(char * cmdbuf[], int * index); // check if the command contains redirection symbols

int Pipelining(char * vec1[], char* vec2[]); 

int Redirection(char * cmdbuf[]);


// ********** Main **********************

int main(int argc, char ** argv)
{

	while(1)
	{
		int i=1, j=0;

		char * vec1[32];	// command 1
		char cmdbuf[256];	// command line input
		char path[1024];	// current path
		int pipeCheck=0;	// see if the command line input includes a pipe

		getcwd(path,1024);

		//printf("My_Shell_$ : %s ", path);  

		printf("My_Shell_$: ");		// Print the command prompt

		fflush(stdout);

		fgets(cmdbuf, 256, stdin); 	// Get command line input from the user

		cmdbuf[strlen(cmdbuf)-1] = '\0';

		if(strcmp(cmdbuf, "exit") == 0) // exit the program if "exit" is entered
		{
			printf("Terminate the program because exit was entered \n");

			return 0;
		}
			

		vec1[0] = strtok(cmdbuf, " ");  // get the 1st command by tokenizing with a white space

		while(1)		 	// store the leftovers in the next command

			if( (vec1[i++] = strtok(NULL, " ")) == NULL)
				break;			// exit if there is no leftover

		// ********* If there is a pipe symbol "|" in the command prompt ************

		if( (pipeCheck = check_Pipe(vec1)) != 0)
		{
			char * vec2[32];	// 2nd command	
			vec1[pipeCheck] = NULL;
			
			for(i=0, j= pipeCheck+1; vec1[j] != NULL; i++, j++)
			{
				vec2[i] = (char *) malloc(strlen(vec1[j])+1);
				strcpy(vec2[i], vec1[j]);	// copy
				vec1[j] = NULL;
			}
			
			vec2[i] = NULL;

			if(fork() == 0)
				Pipelining(vec1,vec2);
			
			wait();
			continue;
		}

		// ******** If there is a redirection symbol ***********************

		if(Redirection(vec1) != 0)
			continue;

		if(fork() == 0)
		{
			execvp(vec1[0], vec1);
		}

		wait();

	}	

	printf("Start !!!!!!!!!!11\n");

	return 0;	

}


// *** Check to see if the command prompt from the user contains a pipe symbol "|"

int check_Pipe(char * cmdbuf[])
{
	int i=0;

	// Search the array for the command line input

	for(i=1; cmdbuf[i] != NULL; i++)
	{
		if(strcmp(cmdbuf[i], "|") == 0) // if there is a pipe symbol "|"
			return i; 		// return the location
	}

	return 0;	// return 0, if there is no pipe symbol

}

// *** Check if the command contains redirection symbols. "index" stores the mode of redirection

int check_Redirection(char * cmdbuf[], int * index)
{
	int i=0;

	*index = -1;

	// Search the array for the command line input

	for(i=1; cmdbuf[i] != NULL; i++)
	{
		if(strcmp(cmdbuf[i], ">") == 0)
		{
			*index = i;
			return 1;
		}

		if(strcmp(cmdbuf[i], ">>") == 0)
		{
			*index = i;
			return 2;
		}

		if(strcmp(cmdbuf[i], "<") == 0)
		{
			*index = i;
			return 3;
		}

	}


	return 0; // no redirection symbol found
}


// *** Implement pipelining with respect to the two commands: vec1, vec2

int Pipelining(char * vec1[], char * vec2[])
{
	int fds[2];

	// Create Pipe

	if(pipe(fds) < 0)
	{
		perror("pipe error");
		exit(0);
	}

	// Create a child process
	// Stop the process's standard output and send the output to pipe

	if(fork() == 0)
	{
		close(1);	// close the standard output
		dup(fds[1]);	// redirect the output

		close(fds[0]);
		close(fds[1]);

		execvp(vec1[0], vec1);
		exit(0);
	}

	// Create a child process
	// Stop the process's standard input and receive the input from the pipe 

	if(fork() == 0)
	{
		close(0);	// close the standard inpuit
		dup(fds[0]);	// redirect the input

		close(fds[0]);
		close(fds[1]);

		execvp(vec2[0], vec2);
		exit(0);
	}
	close(fds[0]);
	close(fds[1]);

	while(wait(NULL) > 0)

	return 0;
}


// *** Implement Redirection

int Redirection(char * cmdbuf[])
{
	int fd, i;

	int mode, index;

	int cfd1, cfd2;

	char * cmd[32];

	mode = check_Redirection(cmdbuf, &index); // Check if there's a redirection

	for(i=0; i<index; i++)
	{
		cmd[i] = (char *) malloc(sizeof(char*)*10);

		strcpy(cmd[i], cmdbuf[i]);
	}

	if(mode == 0)		// there is no redirection
		return 0;


	if(fork() == 0)
	{
		switch(mode)
		{
			case 1: // '>' redirection

				close(1);	// close the standard output

				if(fd = open(cmdbuf[index+1], O_WRONLY | O_CREAT | O_TRUNC, 0644) == -1) 
				{
					// Delete the existing file and create a new file 
					printf("Failed to write file: Case > \n");
					exit(1);
				}
			
				dup(fd);	// redirect the output

				close(fd);

				execvp(cmd[0], cmd);

				break;

			case 2: // '>>' redirection

				close(1);	// close the standard output

				if(fd = open(cmdbuf[index+1], O_WRONLY | O_CREAT | O_APPEND, 0644) == -1) 
				{
					// Append to the existing file 
					printf("Failed to write file: Case >> \n");
					exit(1);
				}

				dup(fd);	// redirect the output
			
				close(fd);

				execvp(cmd[0], cmd);

				break;

			default: // '<' redirection

				close(0);	// close the standard input

				if(fd = open(cmdbuf[index+1], O_RDONLY) == -1) 
				{
					// Open the file in read mode 
					printf("Failed to read file: Case < \n");
				}

				dup(fd);	// redirect the input

				close(fd);

				execvp(cmd[0], cmd);	
			
		}
	}

	wait();

	return 1;

}










