#ifndef GUARD_MALLOC_H
#define GUARD_MALLOC_H

#include "global.h"

// The final 8 KiB are reserved for the HQ mixer code and mix buffer.
// HQMixerInitHeap also limits vanilla ROM callers using the original size.
#define HEAP_SIZE 0x1A000
#define malloc Alloc
#define calloc(ct, sz) AllocZeroed((ct) * (sz))
#define free Free

#define FREE_AND_SET_NULL(ptr)          \
{                                       \
    free(ptr);                          \
    ptr = NULL;                         \
}


extern u8 gHeap[];
void *Alloc(u32 size);
void __attribute__((long_call)) *AllocZeroed(u32 size);
void Free(void *pointer);
void __attribute__((long_call)) InitHeap(void *pointer, u32 size);

void __attribute__((long_call)) *AllocZeroed(u32 size);
void *AllocZeroed(u32 size);

#endif // GUARD_MALLOC_H
