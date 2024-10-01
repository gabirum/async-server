#include "hashtable.h"

#include <mimalloc.h>

#define NOT_FOUND_INDEX ((size_t) - 1)
#define GET_INDEX(hash, cap) (hash % cap)

hashtable_t *ht_new(size_t initial_capacity, float factor)
{
  hashtable_t *table = mi_malloc_small(sizeof(hashtable_t));
  if (table == NULL)
  {
    return NULL;
  }

  ht_entry_t *entries = mi_calloc(initial_capacity, sizeof(ht_entry_t));
  if (entries == NULL)
  {
    mi_free(table);
    return NULL;
  }
  table->entries = entries;
  table->capacity = initial_capacity;
  table->factor = factor;
  table->length = 0;

  return table;
}

void ht_delete(hashtable_t *table, ht_cleanup_f clean)
{
  for (size_t i = 0; i < table->capacity; i++)
  {
    ht_entry_t *entry = &table->entries[i];
    if (entry->key != NULL)
    {
      string_delete(entry->key);
      if (clean != NULL)
      {
        clean(entry->data);
      }
    }
  }
  mi_free(table->entries);
  mi_free(table);
}

static void _ht_set(ht_entry_t *entries, size_t capacity, ht_entry_t *entry)
{
  size_t index = GET_INDEX(string_hash(entry->key), capacity);
  ht_entry_t *current_entry = &entries[index];

  for (size_t i = 0; i < capacity && current_entry->key != NULL; i++)
  {
    index = index + 1;
    if (index >= capacity)
    {
      index = 0;
    }
    current_entry = &entries[index];
  }

  *current_entry = *entry;
}

static bool _ht_expand(hashtable_t *table)
{
  size_t new_capacity = table->capacity + (table->capacity >> 1);
  new_capacity = new_capacity == table->capacity ? 2 * new_capacity : new_capacity;
  if (new_capacity < table->capacity)
  {
    return false;
  }

  ht_entry_t *new_entries = mi_calloc(new_capacity, sizeof(ht_entry_t));
  if (new_entries == NULL)
  {
    return false;
  }

  for (size_t i = 0; i < table->capacity; i++)
  {
    ht_entry_t *current_entry = &table->entries[i];
    if (current_entry->key == NULL)
    {
      continue;
    }

    _ht_set(new_entries, new_capacity, current_entry);
  }

  mi_free(table->entries);
  table->entries = new_entries;
  table->capacity = new_capacity;

  return true;
}

static size_t _ht_find(hashtable_t *table, string_t *key)
{
  size_t index = GET_INDEX(string_hash(key), table->capacity);

  ht_entry_t current_entry = table->entries[index];
  for (size_t i = 0; i < table->length && current_entry.key != NULL; i++)
  {
    if (string_equal(current_entry.key, key))
    {
      return index;
    }

    if (++index >= table->capacity)
    {
      index = 0;
    }
    current_entry = table->entries[index];
  }

  return NOT_FOUND_INDEX;
}

bool ht_set(hashtable_t *table, string_t *key, void *data)
{
  size_t index = _ht_find(table, key);
  if (index != NOT_FOUND_INDEX)
  {
    ht_entry_t *found_entry = &table->entries[index];
    found_entry->data = data;
    return true;
  }

  string_t *ht_key = string_copy(key);
  if (ht_key == NULL)
  {
    return false;
  }

  if (table->length >= (table->capacity * table->factor))
  {
    if (!_ht_expand(table))
    {
      string_delete(ht_key);
      return false;
    }
  }

  ht_entry_t entry;
  entry.key = ht_key;
  entry.data = data;
  _ht_set(table->entries, table->capacity, &entry);
  table->length++;

  return true;
}

bool ht_has(hashtable_t *table, string_t *key)
{
  return _ht_find(table, key) != NOT_FOUND_INDEX;
}

ht_entry_t *ht_get(hashtable_t *table, string_t *key)
{
  size_t index = _ht_find(table, key);
  if (index == NOT_FOUND_INDEX)
  {
    return NULL;
  }

  return &table->entries[index];
}

hashtable_it_t ht_iterator(hashtable_t *table)
{
  hashtable_it_t it;
  it.index = 0;
  it.entry = &table->entries[0];
  it.table = table;

  return it;
}

ht_entry_t const *hti_get(hashtable_it_t *it)
{
  return it->entry;
}

bool hti_next(hashtable_it_t *it)
{
  while (it->index < it->table->capacity)
  {
    size_t i = it->index++;
    ht_entry_t *entry = &it->table->entries[i];
    if (entry->key != NULL)
    {
      it->entry = entry;
      return true;
    }
  }

  return false;
}
