// Copyright 2022 Alexey Kutepov <reximkut@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:

// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef ARENA_H_
#define ARENA_H_

#include <stddef.h>
#include <stdint.h>

#ifndef ARENA_ASSERT
#include <assert.h>
#define ARENA_ASSERT assert
#endif

#define ARENA_BACKEND_LIBC_MALLOC 0
#define ARENA_BACKEND_LINUX_MMAP 1
#define ARENA_BACKEND_WIN32_VIRTUALALLOC 2
#define ARENA_BACKEND_WASM_HEAPBASE 3

#ifndef ARENA_BACKEND
#define ARENA_BACKEND ARENA_BACKEND_LIBC_MALLOC
#endif // ARENA_BACKEND

typedef struct Region Region;

struct Region {
    Region *next;
    size_t count;
    size_t capacity;
    uintptr_t data[];
};

typedef struct {
    Region *begin, *end;
} Arena;

#define REGION_DEFAULT_CAPACITY (8*1024)

Region *new_region(size_t capacity);
void free_region(Region *r);

// TODO: snapshot/rewind capability for the arena
// - Snapshot should be combination of a->end and a->end->count.
// - Rewinding should be restoring a->end and a->end->count from the snapshot and
// setting count-s of all the Region-s after the remembered a->end to 0.
void *arena_alloc(Arena *a, size_t size_bytes);
void *arena_realloc(Arena *a, void *oldptr, size_t oldsz, size_t newsz);

void arena_reset(Arena *a);
void arena_free(Arena *a);

int arena_slurp_file(Arena *arena, const char *file_path_cstr, String_View *content);
String_View arena_sv_concat(Arena *arena, ...);
const char *arena_cstr_concat(Arena *arena, ...);
const char *arena_sv_to_cstr(Arena *arena, String_View sv);
String_View arena_sv_dup(Arena *arena, String_View sv);

#define SV_CONCAT(arena, ...)                   \
    arena_sv_concat(arena, __VA_ARGS__, SV_NULL)

#define CSTR_CONCAT(arena, ...)                 \
    arena_cstr_concat(arena, __VA_ARGS__, NULL)

#endif // ARENA_H_

#ifdef ARENA_IMPLEMENTATION

#if ARENA_BACKEND == ARENA_BACKEND_LIBC_MALLOC
#include <stdlib.h>

// TODO: instead of accepting specific capacity new_region() should accept the size of the object we want to fit into the region
// It should be up to new_region() to decide the actual capacity to allocate
Region *new_region(size_t capacity)
{
    size_t size_bytes = sizeof(Region) + sizeof(uintptr_t)*capacity;
    // TODO: it would be nice if we could guarantee that the regions are allocated by ARENA_BACKEND_LIBC_MALLOC are page aligned
    Region *r = malloc(size_bytes);
    ARENA_ASSERT(r);
    r->next = NULL;
    r->count = 0;
    r->capacity = capacity;
    return r;
}

void free_region(Region *r)
{
    free(r);
}
#elif ARENA_BACKEND == ARENA_BACKEND_LINUX_MMAP
#  error "TODO: Linux mmap backend is not implemented yet"
#elif ARENA_BACKEND == ARENA_BACKEND_WIN32_VIRTUALALLOC

#if !defined(_WIN32)
#  error "Current platform is not Windows"
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define INV_HANDLE(x)       (((x) == NULL) || ((x) == INVALID_HANDLE_VALUE))

Region *new_region(size_t capacity)
{
    SIZE_T size_bytes = sizeof(Region) + sizeof(uintptr_t) * capacity;
    Region *r = VirtualAllocEx(
        GetCurrentProcess(),      /* Allocate in current process address space */
        NULL,                     /* Unknown position */
        size_bytes,               /* Bytes to allocate */
        MEM_COMMIT | MEM_RESERVE, /* Reserve and commit allocated page */
        PAGE_READWRITE            /* Permissions ( Read/Write )*/
    );
    if (INV_HANDLE(r))
        ARENA_ASSERT(0 && "VirtualAllocEx() failed.");

    r->next = NULL;
    r->count = 0;
    r->capacity = capacity;
    return r;
}

void free_region(Region *r)
{
    if (INV_HANDLE(r))
        return;

    BOOL free_result = VirtualFreeEx(
        GetCurrentProcess(),        /* Deallocate from current process address space */
        (LPVOID)r,                  /* Address to deallocate */
        0,                          /* Bytes to deallocate ( Unknown, deallocate entire page ) */
        MEM_RELEASE                 /* Release the page ( And implicitly decommit it ) */
    );

    if (FALSE == free_result)
        ARENA_ASSERT(0 && "VirtualFreeEx() failed.");
}

#elif ARENA_BACKEND == ARENA_BACKEND_WASM_HEAPBASE
#  error "TODO: WASM __heap_base backend is not implemented yet"
#else
#  error "Unknown Arena backend"
#endif

// TODO: add debug statistic collection mode for arena
// Should collect things like:
// - How many times new_region was called
// - How many times existing region was skipped
// - How many times allocation exceeded REGION_DEFAULT_CAPACITY

void *arena_alloc(Arena *a, size_t size_bytes)
{
    size_t size = (size_bytes + sizeof(uintptr_t) - 1)/sizeof(uintptr_t);

    if (a->end == NULL) {
        ARENA_ASSERT(a->begin == NULL);
        size_t capacity = REGION_DEFAULT_CAPACITY;
        if (capacity < size) capacity = size;
        a->end = new_region(capacity);
        a->begin = a->end;
    }

    while (a->end->count + size > a->end->capacity && a->end->next != NULL) {
        a->end = a->end->next;
    }

    if (a->end->count + size > a->end->capacity) {
        ARENA_ASSERT(a->end->next == NULL);
        size_t capacity = REGION_DEFAULT_CAPACITY;
        if (capacity < size) capacity = size;
        a->end->next = new_region(capacity);
        a->end = a->end->next;
    }

    void *result = &a->end->data[a->end->count];
    a->end->count += size;
    return result;
}

void *arena_realloc(Arena *a, void *oldptr, size_t oldsz, size_t newsz)
{
    if (newsz <= oldsz) return oldptr;
    void *newptr = arena_alloc(a, newsz);
    char *newptr_char = newptr;
    char *oldptr_char = oldptr;
    for (size_t i = 0; i < oldsz; ++i) {
        newptr_char[i] = oldptr_char[i];
    }
    return newptr;
}

void arena_reset(Arena *a)
{
    for (Region *r = a->begin; r != NULL; r = r->next) {
        r->count = 0;
    }

    a->end = a->begin;
}

void arena_free(Arena *a)
{
    Region *r = a->begin;
    while (r) {
        Region *r0 = r;
        r = r->next;
        free_region(r0);
    }
    a->begin = NULL;
    a->end = NULL;
}

// TODO: Don't allocate memory in the arena to convert the file_path to a zero-terminated string.
// Instead, why don't we just accept a cstring here? Most of the time when we want to open a file,
// the input came from command line arguments, which are zero-terminated.
int arena_slurp_file(Arena *arena, const char *file_path_cstr, String_View *content)
{
    FILE *f = fopen(file_path_cstr, "rb");
    if (f == NULL) {
        return -1;
    }

    if (fseek(f, 0, SEEK_END) < 0) {
        return -1;
    }

    long m = ftell(f);
    if (m < 0) {
        return -1;
    }

    char *buffer = arena_alloc(arena, (size_t) m);
    if (buffer == NULL) {
        return -1;
    }

    if (fseek(f, 0, SEEK_SET) < 0) {
        return -1;
    }

    size_t n = fread(buffer, 1, (size_t) m, f);
    if (ferror(f)) {
        return -1;
    }

    fclose(f);

    if (content) {
        content->count = n;
        content->data = buffer;
    }

    return 0;
}

String_View arena_sv_concat(Arena *arena, ...)
{
    size_t len = 0;

    va_list args;
    va_start(args, arena);
    String_View sv = va_arg(args, String_View);
    while (sv.data != NULL) {
        len += sv.count;
        sv = va_arg(args, String_View);
    }
    va_end(args);

    char *buffer = arena_alloc(arena, len);
    len = 0;

    va_start(args, arena);
    sv = va_arg(args, String_View);
    while (sv.data != NULL) {
        memcpy(buffer + len, sv.data, sv.count);
        len += sv.count;
        sv = va_arg(args, String_View);
    }
    va_end(args);

    return (String_View) {
        .count = len,
        .data = buffer
    };
}

String_View arena_sv_dup(Arena *arena, String_View sv)
{
    char *buffer = arena_alloc(arena, sv.count);
    memcpy(buffer, sv.data, sv.count);
    return (String_View) {
        .count = sv.count,
        .data = buffer,
    };
}

const char *arena_sv_to_cstr(Arena *arena, String_View sv)
{
    char *cstr = arena_alloc(arena, sv.count + 1);
    memcpy(cstr, sv.data, sv.count);
    cstr[sv.count] = '\0';
    return cstr;
}

#endif // ARENA_IMPLEMENTATION
