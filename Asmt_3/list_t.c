/* Thomas Foulds (tcf9bj)
 * 02/18/13
 * CS4414
 * list_t.c REV. 2
 */

#include <stdio.h>
#include <stdlib.h>
#include "list_t.h"

void list_init(list_t *l,
	       int (*compare)(const void *key, const void *with),
	       void (*data_delete)(void *data)){
  l->head = NULL;
  l->tail = NULL;
  l->length = 0;
  l->compare = compare;
  l->data_delete = data_delete;
};

void list_visit_items(list_t *l, void (*visitor)(void *v)){
  list_item_t *current_item = l->head;
  while(current_item != NULL)
    {
      visitor(current_item);
      current_item = current_item->next;
    }
};

void list_insert_tail(list_t *l, void *v){

  list_item_t *new = malloc(sizeof(list_item_t));

  if(new == NULL)
    {
      perror("malloc failed to create new object.");
      exit(-1);
    }

  list_item_init(new, v);

  if( l->length == 0 )
    {
      l->head = new;
      l->tail = new;
    }
  else
    {
      l->tail->next = new;
      new->prev = l->tail;
      l->tail = new;
    }
  l->length++;
};

void list_insert_sorted(list_t *l, void *v){
  list_insert_tail(l,v);
  // This is heinous and I know it, but it is literally the last idea I have
  list_item_t *items[l->length];
  list_item_t *current_item = l->head;
  unsigned int i;
  // We're going to get pointers to every object in the list, put them in an array, sort the array, and then retie all the pointers
  for(i = 0; i < l->length; i++)
    {
      items[i] = current_item;
      current_item = current_item->next;
    }
  // Sort our items according to the sort function
  qsort(items, l->length, sizeof(list_item_t*),l->compare);
  // Retie the pointers
  
  l->head = items[0];
  l->head->prev = NULL;
  l->tail = items[(l->length) -1];
  l->tail->next = NULL;

  if(l->length == 2)
    {
      l->head->next = l->tail;
      l->tail->prev = l->head;
    }
  if(l->length > 2)
    {
      l->head->next = items[1];
      l->tail->prev = items[(l->length) - 2];
      unsigned int j;
      for(j = 1; j < (l->length) - 1; j++)
	{
	  items[j]->next = items[j+1];
	  items[j+1]->prev = items[j];
	}
    }
};

void list_remove_head(list_t *l){
  if(l->head != NULL)
    {
      list_item_t *new_head = l->head->next;
      l->data_delete(l->head);
      free(l->head);
      l->length --;
      l->head = new_head;
    }
  else
    {
      printf("You cannot remove from an empty list.\n");
    }
};

void list_rotate_one(list_t *l){
  printf("== list_rotate_one ==\n");
  if(l->length > 1){
    list_item_t *old_head = l->head;
    l->head = old_head->next;
    printf("New head: %p\n", l->head);
    l->head->prev = NULL;
    old_head->prev = l->tail;
    old_head->next = NULL;
    l->tail->next = old_head;
    l->tail = old_head;
  }
  printf("!= list_rotate_one =!\n");
};
