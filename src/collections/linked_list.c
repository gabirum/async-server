#include "linked_list.h"

#include <mimalloc.h>

linked_list_t *ll_new()
{
  return mi_zalloc_small(sizeof(linked_list_t));
}

void ll_delete(linked_list_t *list, ll_cleanup_f clean)
{
  if (list == NULL)
    return;

  linked_list_node_t *node = list->head;
  while (node != NULL)
  {
    linked_list_node_t *next_node = node->next;
    if (clean != NULL)
      clean(node->data);
    mi_free(node);
    node = next_node;
  }
  mi_free(list);
}

#define ALLOC_NODE(varname, data)                                            \
  linked_list_node_t *varname = mi_zalloc_small(sizeof(linked_list_node_t)); \
  if (varname == NULL)                                                       \
  {                                                                          \
    return false;                                                            \
  }                                                                          \
  varname->data = data;

#define PUSH_IF_ZERO(list, node)    \
  if (list->length == 0)            \
  {                                 \
    list->head = list->tail = node; \
    list->length++;                 \
    return true;                    \
  }

bool ll_push_front(linked_list_t *list, void *data)
{
  ALLOC_NODE(node, data)

  PUSH_IF_ZERO(list, node)

  node->next = list->head;
  list->head = node;
  list->length++;

  return true;
}

bool ll_push_back(linked_list_t *list, void *data)
{
  ALLOC_NODE(node, data)

  PUSH_IF_ZERO(list, node)

  list->tail->next = node;
  list->tail = node;
  list->length++;

  return true;
}

#define INDEX_GUARD(index, list) \
  if (index >= list->length)     \
    return NULL;

void *ll_pull(linked_list_t *list, size_t index)
{
  INDEX_GUARD(index, list)

  linked_list_node_t *node = list->head;
  for (size_t i = 0; i < index; i++)
    node = node->next;

  return node->data;
}

void *ll_pull_front(linked_list_t *list)
{
  if (list->length == 0)
    return NULL;

  return list->head->data;
}

void *ll_pull_back(linked_list_t *list)
{
  if (list->length == 0)
    return NULL;

  return list->tail->data;
}

void *ll_remove(linked_list_t *list, size_t index)
{
  INDEX_GUARD(index, list);

  if (index == 0)
    return ll_remove_front(list);

  if (index == list->length - 1)
    return ll_remove_back(list);

  linked_list_node_t *old_node = list->head;
  linked_list_node_t *node = list->head->next;
  for (size_t i = 1; i < index; i++)
  {
    old_node = node;
    node = node->next;
  }
  old_node->next = node->next;

  void *data = node->data;

  mi_free(node);

  return data;
}

#define REMOVE_GUARD(list) \
  if (list->length == 0)   \
    return NULL;

void *ll_remove_front(linked_list_t *list)
{
  REMOVE_GUARD(list)

  void *data = list->head->data;
  linked_list_node_t *next = list->head->next;
  mi_free(list->head);

  if (list->length == 1)
  {
    list->head = NULL;
    list->tail = NULL;
  }
  else
  {
    list->head = next;
  }
  list->length--;

  return data;
}

void *ll_remove_back(linked_list_t *list)
{
  REMOVE_GUARD(list)

  void *data = list->tail->data;
  mi_free(list->tail);
  if (list->length == 1)
  {
    list->head = NULL;
    list->tail = NULL;
  }

  linked_list_node_t *node = list->head;
  while (node->next != list->tail)
    node = node->next;
  list->tail = node;

  return data;
}

linked_list_it ll_iterator(linked_list_t *list)
{
  linked_list_it it;
  it.current_node = NULL;
  it.head = list->head;
  return it;
}

bool lli_next(linked_list_it *it)
{
  if (it->current_node == NULL)
    it->current_node = it->head;
  else
    it->current_node = it->current_node->next;
  return it->current_node != NULL;
}

void *lli_get(linked_list_it it)
{
  return it.current_node->data;
}
