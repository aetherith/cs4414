/* Thomas Foulds
 * tcf9bj
 * 01/23/13
 * CS4414
 * list_t.h
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

void list_visit_items(list_t *l, void (*visitor)(void *v));

void list_insert_tail(list_t *l, void *v);

void list_insert_sorted(list_t *l, void *v);

void list_remove_head(list_t *l);

#endif
