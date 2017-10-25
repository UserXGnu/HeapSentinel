#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "heap_sentinel.h"
#include "sentinel_config.h"

bool app_alloc = off;

extern void* __libc_malloc(size_t size);
int malloc_hook_active = 1;
FILE* al_heap_log = NULL;

extern void __libc_free(void* ptr);
static int free_hook_active = 1;
static FILE* fr_heap_log = NULL;

static int state = 0;

int allocs = 0;

static void
sentinel_err_msg(const char* errmsg) {
#ifdef DEBUG
    assert(errmsg != NULL);
#endif

    char err[ERR_LEN];

    strncat(err, (const char*)ERR_PREFIX_MSG, sizeof(err));

    strncat(err, errmsg, sizeof(err) - strlen(err));
    strncat(err, " !!\n", sizeof(err) - strlen(err));
    puts(err);
}

static void
sentinel_err_quit(const char* errmsg) {
#ifdef DEBUG
    assert(errmsg != NULL);
#endif

    char err[ERR_LEN];

    strncat(err, (const char*)ERR_PREFIX_MSG, sizeof(err));

    strncat(err, errmsg, sizeof(err) - strlen(err));
    strncat(err, " !!\n", sizeof(err) - strlen(err));
    puts(err);

    exit(EXIT_FAILURE);
}


void*
malloc(size_t size) {
    void* caller = __builtin_return_address(0);

    if (malloc_hook_active) return wp_malloc_hook(size, caller);

    return __libc_malloc(size);
}

void*
wp_malloc_hook(size_t size, const void* caller) {
    void* ptr;

    malloc_hook_active = 0;

    state = (state != 2 ? 1 : state);
    if (state == 1) allocs++;
    ptr = malloc(size);
    if (!first_alloc) {
        al_heap_log = fopen(al_heap_log_path, "w+");
        if (!al_heap_log) sentinel_err_quit("unable to open heap log file");

        first_alloc = true;
    }

    /* fprintf(heap_log, "malloc (%u) from %p returning %p\n", (unsigned
     * int)size, */
    /*         caller, ptr); */

    /*     if (state == 1) printf("ptr: %p\n", ptr); */
    // printf("\t\t%p\n", ptr);
    if (state == 1 && app_alloc) fprintf(al_heap_log, "%p\n", ptr);

    malloc_hook_active = 1;

    state = (state == 1 ? 0 : state);

    return ptr;
}

void
free(void* ptr) {
    void* caller = __builtin_return_address(0);

    if (free_hook_active) wp_free(ptr, caller);

    __libc_free(ptr);
}

void
wp_free(void* ptr, const void* caller) {
    if (!first_alloc) return;

    state = (state != 1 ? 2 : state);
    if (!fr_heap_log) {
        fr_heap_log = fopen(fr_heap_log_path, "w+");
        if (!fr_heap_log)
            sentinel_err_quit("unable to open freed heap log file");

        first_free = true;
    }
    //    printf("ptr: %p\n", ptr);
    /* fprintf(fr_heap_log, "free (%p) from %p\n", ptr, caller); */
    if (state == 2 && app_alloc) fprintf(fr_heap_log, "%p\n", ptr);

    free_hook_active = 0;

    free(ptr);

    free_hook_active = 1;

    state = (state == 2 ? 0 : state);
}

void
restart_pointers(void) {
    if (fr_heap_log) {
        fseek(fr_heap_log, SEEK_SET, 0);
        fflush(fr_heap_log);
    }
    if (al_heap_log) {
        fseek(al_heap_log, SEEK_SET, 0);
        fflush(al_heap_log);
    }
}

void
leak_verifier(void) {
    char al_line[64];
    char fr_content[4098];

    int i = 0;
    int c = 0;
    if (!al_heap_log) return;

    // restart_pointers();
    printf("Allocs: %d\n", allocs);

    fseek(al_heap_log, SEEK_SET, 0);
    if (!fr_heap_log) {
        puts(
            "WARNING:\n"
            "\tThe following addresses were allocated\n"
            "\tbut were not freed [!!] (memory leaking)\n");

        while (fscanf(al_heap_log, "%s", al_line) != EOF) {
            printf("\t%s (leaked)\n", al_line);
            memset(al_line, 0x00, sizeof(al_line));
        }

        return;
    }

    fseek(fr_heap_log, SEEK_SET, 0);
    while ((c = fgetc(fr_heap_log)) != 0x00 && c != EOF) {
        fr_content[i] = c;
        i++;
    }

    /* printf("FR:\n%s\n", fr_content); */

    i = 0;
    while ((c = fgetc(al_heap_log)) != 0x00 && c != EOF) {
        if (c == '\n') {
            printf("Searching: %s\n", al_line);
            i = 0;

            if (!strstr(fr_content, al_line)) {
                printf(
                    "WARNING: memory allocated @ %s and not freed, memory "
                    "leaked [!!]\n",
                    al_line);
            }
            continue;
        }
        al_line[i] = c;
        i++;
    }
}
