#include "hash.h"

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

uint64_t fnv_hash(char const *string, size_t length)
{
  uint64_t hash = FNV_OFFSET;
  for (size_t i = 0; i < length; i++)
  {
    hash ^= (uint8_t)string[i];
    hash *= FNV_PRIME;
  }
  return hash;
}
