#if !defined(_HASH_H_)
#define _HASH_H_

#include <stddef.h>
#include <stdint.h>

uint64_t fnv_hash(char const *string, size_t length);

#endif // _HASH_H_
