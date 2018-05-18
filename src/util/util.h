#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

void random_bytes(void* buf, size_t nbytes);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* UTIL_H */
