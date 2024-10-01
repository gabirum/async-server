#include "string.h"

#include <stdio.h>
#include <stdarg.h>

#include <mimalloc.h>

#include "../utils/hash.h"

string_t string_from(char *data)
{
  string_t str;
  str.data = data;
  str.length = strlen(data);
  str.hashcode = 0;
  return str;
}

string_t *string_new(char const *data, size_t size)
{
  if (size == 0)
    return NULL;

  string_t *str = mi_malloc_small(sizeof(string_t));
  if (str == NULL)
    return NULL;

  char *string = mi_strndup(data, size);
  if (string == NULL)
  {
    mi_free(str);
    return NULL;
  }

  str->length = size;
  str->hashcode = 0;
  str->data = string;

  return str;
}

string_t *string_new_format(char const *format, ...)
{
  va_list args;
  va_start(args, format);
  int length = vsnprintf(NULL, 0, format, args);
  va_end(args);

  if (length <= 0)
    return NULL;

  string_t *string = mi_malloc_small(sizeof(string_t));
  if (string == NULL)
    return NULL;

  char *data = mi_calloc(length + 1, sizeof(char));
  if (data == NULL)
  {
    mi_free(string);
    return NULL;
  }

  va_start(args, format);
  vsnprintf(data, length, format, args);
  va_end(args);

  string->data = data;
  string->length = length;
  string->hashcode = 0;

  return string;
}

void string_delete(string_t *str)
{
  if (str == NULL)
    return;

  mi_free(str->data);
  mi_free(str);
}

string_t *string_copy(string_t const *src)
{
  if (src->length == 0)
    return NULL;

  string_t *dest = mi_malloc_small(sizeof(string_t));
  if (dest == NULL)
    return NULL;

  char *copied_data = mi_strndup(src->data, src->length);
  if (copied_data == NULL)
  {
    mi_free(dest);
    return NULL;
  }

  *dest = *src;
  dest->data = copied_data;

  return dest;
}

bool string_cstr_concatn(string_t *str, char const *src, size_t size)
{
  if (size == 0)
    return true;

  size_t const new_size = str->length + size + 1;
  if (str->data == src)
  {
    char *new_data = mi_malloc(new_size);
    if (new_data == NULL)
      return false;

    memcpy(new_data, str->data, str->length);
    memcpy(new_data + str->length, src, size);
    str->data = new_data;
  }
  else
  {
    char *new_space = mi_realloc(str->data, new_size);
    if (new_space == NULL)
      return false;

    str->data = new_space;
    memcpy(str->data + str->length, src, size);
  }

  str->length += size;
  str->data[str->length] = 0;
  str->hashcode = 0;

  return true;
}

bool string_concat(string_t *str, string_t const *src)
{
  char *src_data = src->data;

  bool result = string_cstr_concatn(str, src_data, src->length);
  if (result && str == src)
    mi_free(src_data);

  return result;
}

bool string_cstr_concat(string_t *str, char const *src)
{
  char *old_str_data = str->data;
  bool result = string_cstr_concatn(str, src, strlen(src));
  if (result && old_str_data == src)
    mi_free(old_str_data);

  return result;
}

uint64_t string_hash(string_t *str)
{
  if (str->hashcode == 0)
    str->hashcode = fnv_hash(str->data, str->length);

  return str->hashcode;
}

bool string_equal(string_t *a, string_t *b)
{
  if (a == b)
    return true;

  if (a->length != b->length || string_hash(a) != string_hash(b))
    return false;

  return strncmp(a->data, b->data, a->length) == 0;
}
