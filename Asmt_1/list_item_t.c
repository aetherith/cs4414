/* Thomas Foulds
 * tcf9bj
 * 01/23/13
 * CS4414
 * list_item_t.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list_item_t.h"

void list_item_init(list_item_t *i, void *data){
  i->prev = NULL;
  i->next = NULL;
  i->data = data;
};

void string_data_delete(void *item){
  free(((list_item_t*)item)->data);
};

void print_data_string(void *v){
  printf("%s\n", (char*)( ( (list_item_t*)v)->data));
};

int list_item_compare_string(const void *key, const void *with){
  list_item_t **arg1 = (list_item_t**)key;
  list_item_t **arg2 = (list_item_t**)with;
  char *data1 = (char*)((*arg1)->data);
  char *data2 = (char*)((*arg2)->data);
  return strcmp(data1, data2);
};
