/* Thomas Foulds
 * tcf9bj
 * 01/23/13
 * CS4414
 * list_item_t.h
 */

#ifndef LIST_ITEM_T_H
#define LIST_ITEM_T_H

typedef struct list_item {
  struct list_item *prev, *next;
  void *data
} list_item_t;

void list_item_init(list_item_t *i, void *data);

// Example of data_delete function
// void string_data_delete(void *item);

// Example of visitor function
// void print_data_string(void *v);

// Example of comparision function
// int list_item_compare_string(const void *key, const void *with);

#endif
