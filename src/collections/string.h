#if !defined(_STRING_H_)
#define _STRING_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct string
{
  size_t length;
  uint64_t hashcode;
  char *data;
} string_t;

#define STRING(ptr) ((string_t *)ptr)

string_t string_from(char *data);
string_t *string_new(char const *data, size_t size);
#define string_new_(data) string_new(data, strlen(data))
string_t *string_new_format(char const *format, ...);

void string_delete(string_t *str);

string_t *string_copy(string_t const *str);
bool string_cstr_concatn(string_t *str, char const *src, size_t size);
bool string_concat(string_t *str, string_t const *src);
bool string_cstr_concat(string_t *str, char const *src);

uint64_t string_hash(string_t *str);

bool string_equal(string_t *a, string_t *b);

#endif // _STRING_H_
