/* Thomas Foulds
 * tcf9bj
 * 01/23/13
 * CS4414
 * harness.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "list_t.h"
#include "list_item_t.h"

typedef int bool;
#define true 1
#define false 0

void cleanup_list(list_t *l);

bool read_line(FILE *f, char *buf, int buf_size);

int main(int argc, char* argv[]){
  
  if(argc > 3)
    {
      printf("You may only enter a filename and an option.\necho sort tail tail-remove\nExiting...\n");
      exit(-1);
    }
  else if (argc < 3)
    {
      printf("You must enter both a filename and an option.\necho sort tail tail-remove\nExiting...\n");
      exit(-1);
    }

  // Open the supplied input file read only
  FILE *input_file = fopen(argv[1], "r");
  if(input_file == NULL)
    {
      perror("Please enter a different filename");
      exit(-1);
    }

  // Echo the file
  if(strcmp(argv[2], "echo") == 0)
    {
      char echo_buf[41];
      while(read_line(input_file, echo_buf, 41))
	{
	  printf("%s\n", echo_buf);
	  memset(echo_buf, '\0', 41);
	}
    }

  // Put the file in a list and visit-print the list
  else if(strcmp(argv[2], "tail") == 0)
    {
      list_t list;
      list_init(&list, list_item_compare_string, string_data_delete);

      char *tail_buf = malloc(41*sizeof(char));
      if(tail_buf == NULL)
	{
	  perror("malloc failed to create new buffer.");
	  exit(-1);
	}
      while(read_line(input_file, tail_buf, 41))
	{
	  list_insert_tail(&list, tail_buf);
	  tail_buf = malloc(41*sizeof(char));
	  if(tail_buf == NULL)
	    {
	      perror("malloc failed to create new buffer.");
	      exit(-1);
	    }
	}
      // Free the last buffer created when we looped last
      free(tail_buf);

      list_visit_items(&list, print_data_string);
      cleanup_list(&list);
    }

  // Insert the file into a sorted list ** Does not seem to sort correctly**
  else if(strcmp(argv[2], "sort") == 0)
    {
      list_t list;
      list_init(&list, list_item_compare_string, string_data_delete);
      
      char *sort_buf = malloc(41*sizeof(char));
      if(sort_buf == NULL)
	{
	  perror("malloc failed to create new buffer.");
	  exit(-1);
	}
      while(read_line(input_file, sort_buf, 41))
	{
	  list_insert_sorted(&list, sort_buf);
	  sort_buf = malloc(41*sizeof(char));
	  if(sort_buf == NULL)
	    {
	      perror("malloc failed to create new buffer.");
	      exit(-1);
	    }
	}
      // Free last buffer created as it is unused
      free(sort_buf);

      list_visit_items(&list, print_data_string);
      cleanup_list(&list);
    }

  // Insert the file into a list in order and then remove 3 head nodes until it is empty
  else if(strcmp(argv[2], "tail-remove") == 0)
    {
      printf("Running tail-remove\n");
      list_t list;
      list_init(&list, list_item_compare_string, string_data_delete);

      char *rem_buf = malloc(41*sizeof(char));
      if(rem_buf == NULL)
	{
	  perror("malloc failed to create new buffer.");
	  exit(-1);
	}
      while(read_line(input_file, rem_buf, 41))
	{
	  list_insert_tail(&list, rem_buf);
	  rem_buf = malloc(41*sizeof(char));
	}
      // Free last buffer created
      free(rem_buf);
      
      unsigned int three_sets = (list.length)/3;
      unsigned int i;
      list_visit_items(&list, print_data_string);
      for(i = 0; i < three_sets; i++)
	{
	  list_remove_head(&list);
	  list_remove_head(&list);
	  list_remove_head(&list);
	  printf("----------------------------------------\n");
	  list_visit_items(&list, print_data_string);
	}
      cleanup_list(&list);
      printf("----------------------------------------\n");
      list_visit_items(&list, print_data_string);
    }

  fclose(input_file);
  
  return 0;
};

void cleanup_list(list_t *l){
  while(l->head != NULL)
    {
      list_remove_head(l);
    }
};

bool read_line(FILE *f, char *buf, int buf_size){
  int i = 0;
  for( i = 0; i < buf_size; i++){
    char c = fgetc(f);
    // If we hit the end of the string/line/file but already have put data into the buffer
    if((c == EOF) || (c == ' ') || (c == '\n'))
      {
	if(i > 0) return true;
	else return false;
      }
    // Put the read character in the buffer provided
    buf[i] = c;
  }
  return true;
};
