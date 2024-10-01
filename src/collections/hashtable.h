#if !defined(_HASHTABLE_H_)
#define _HASHTABLE_H_

#include <stddef.h>

#include "string.h"

typedef struct ht_entry
{
  string_t *key;
  void *data;
} ht_entry_t;

typedef struct hashtable
{
  size_t capacity, length;
  float factor;
  ht_entry_t *entries;
} hashtable_t;

typedef void (*ht_cleanup_f)(void *data);

#define HT_DEFAULT_INITIAL_CAPACITY 10
#define HT_DEFAULT_FACTOR 0.75f

hashtable_t *ht_new(size_t initial_capacity, float factor);
void ht_delete(hashtable_t *table, ht_cleanup_f clean);

bool ht_set(hashtable_t *table, string_t *key, void *data);
bool ht_has(hashtable_t *table, string_t *key);
ht_entry_t *ht_get(hashtable_t *table, string_t *key);

typedef struct hashtable_it
{
  size_t index;
  ht_entry_t *entry;
  hashtable_t *table;
} hashtable_it_t;

hashtable_it_t ht_iterator(hashtable_t *table);
ht_entry_t const *hti_get(hashtable_it_t *it);
bool hti_next(hashtable_it_t *it);

#endif // _HASHTABLE_H_
