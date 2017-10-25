#ifndef _HEAP_SENTINEL_H_
#define _HEAP_SENTINEL_H_

#include <stdbool.h>
#include <stdlib.h>
#include <malloc.h>

#define ERR_LEN 128
#define ERR_PREFIX_MSG "[!!] Warning; "

#define on true
#define off false


void *wp_malloc_hook(size_t size, const void *caller);
void wp_free(void *ptr, const void *caller);

void restart_pointers(void);
void leak_verifier(void) __attribute__((destructor));

extern bool app_alloc;

#define HEAP_SENTINEL_TURN(state) app_alloc = state;

void
heap_sentinel_turn(bool status) {
    app_alloc = status;
}

#endif /* end of include guard: _HEAP_SENTINEL_H_ */
