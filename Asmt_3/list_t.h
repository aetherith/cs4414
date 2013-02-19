/* Thomas Foulds (tcf9bj)
 * 02/18/13
 * CS4414
 * list_t.h REV. 2
 */

#ifndef LIST_T_H
#define LIST_T_H

#include "list_item_t.h"

typedef struct list {
  list_item_t *head, *tail;
  unsigned int length;
  int (*compare)(const void *key, const void *with);
  void (*data_delete)(void *data);
} list_t;

void list_init(list_t *l,
	       int (*compare)(const void *key, const void *with),
	       void (*data_delete)(void *data));

// Going from head to tail, visit every node and run *visitor function on the data they contain
void list_visit_items(list_t *l, void (*visitor)(void *v));

// Insert new node at tail of list
void list_insert_tail(list_t *l, void *v);

// Insert new node into an already sorted list
void list_insert_sorted(list_t *l, void *v);

// Delete the head node
void list_remove_head(list_t *l);

// Move the head node to the tail without deleting
void list_rotate_one(list_t *l);

#endif
