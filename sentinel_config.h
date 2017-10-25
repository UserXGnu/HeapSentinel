#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdbool.h>

extern void* malloc(size_t size);
extern void free(void* ptr);
extern void restart_pointers(void);

extern int malloc_hook_active;

static const char* al_heap_log_path = "./al_heap.log";
static const char* fr_heap_log_path = "./fr_heap.log";
static bool first_alloc = false;
static bool first_free = false;

#endif /* end of include guard: _CONFIG_H_ */
