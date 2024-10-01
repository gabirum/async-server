#if !defined(_LINKED_LIST_H_)
#define _LINKED_LIST_H_

#include <stddef.h>
#include <stdbool.h>

typedef struct linked_list_node
{
  void *data;
  struct linked_list_node *next;
} linked_list_node_t;

typedef struct linked_list
{
  linked_list_node_t *head;
  linked_list_node_t *tail;
  size_t length;
} linked_list_t;

typedef void (*ll_cleanup_f)(void *data);

linked_list_t *ll_new();

void ll_delete(linked_list_t *list, ll_cleanup_f clean);

bool ll_push_front(linked_list_t *list, void *data);
bool ll_push_back(linked_list_t *list, void *data);

void *ll_pull(linked_list_t *list, size_t index);
void *ll_pull_front(linked_list_t *list);
void *ll_pull_back(linked_list_t *list);

void *ll_remove(linked_list_t *list, size_t index);
void *ll_remove_front(linked_list_t *list);
void *ll_remove_back(linked_list_t *list);

typedef struct linked_list_it_s
{
  linked_list_node_t *head, *current_node;
} linked_list_it;

linked_list_it ll_iterator(linked_list_t *list);
bool lli_next(linked_list_it *it);
void *lli_get(linked_list_it it);

#endif // _LINKED_LIST_H_
